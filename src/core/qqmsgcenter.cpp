#include "qqmsgcenter.h"

#include <QMetaType>

#include "qqmsglistener.h"
#include "../msgtip.h"

void QQMsgCenter::pushMsg(ShareQQMsgPtr msg)
{
    undispatch_msg_.enqueue(msg);
    parse_done_smp_.release();
}

void QQMsgCenter::run()
{
    while (true)
    {
        parse_done_smp_.acquire();
        lock_.lock();

        ShareQQMsgPtr msg = undispatch_msg_.dequeue();

        switch(msg->type())
        {
        case QQMsg::kBuddiesStatusChange:
        {    
            emit buddiesStateChangeMsgArrive(msg->sendUin(),  msg->status(), msg->client_type());
            break;
        }

        case QQMsg::kFriend:
        case QQMsg::kGroup:
        {
            if (msg->type() == QQMsg::kFriend)
                emit friendChatMsgArrive(msg->talkTo());
            else
                emit groupChatMsgArrive(msg->talkTo());

            bool is_msg_processed = false;
            QQMsgListener *listener;
            foreach(listener, listener_)
            {
                if (msg->talkTo() == listener->id())
                {
                    is_msg_processed = true;
                    emit distributeMsgInMainThread(listener, msg);
                }
            }

            //如果消息没有被处理,发送给MsgTip,并压入old_msg_队列，等待有相应的监听者再次发送
            if (!is_msg_processed)
            {
                msg_tip_->pushMsg(msg);
                old_msg_.enqueue(msg);
            }
        }
            break;

        case QQMsg::kSystem:
            msg_tip_->pushMsg(msg);
            break;

        case QQMsg::kSystemG:
            msg_tip_->pushMsg(msg);
            break;

        default:
            break;
        }
        lock_.unlock();

        writeToSql(msg);
    }
}

void QQMsgCenter::distributeMsg(QQMsgListener *listener, ShareQQMsgPtr msg)
{
    listener->showMsg(msg);
}

void QQMsgCenter::registerListener(QQMsgListener *listener)
{
    lock_.lock();
    QVector<ShareQQMsgPtr> old_msgs(getOldMsg(listener));
    listener->showOldMsg(old_msgs);
    lock_.unlock();
    listener_.append(listener);
}

QVector<ShareQQMsgPtr> QQMsgCenter::getOldMsg(QQMsgListener *listener)
{
    ShareQQMsgPtr msg;
    QVector<ShareQQMsgPtr> msgs;
    foreach(msg, old_msg_)
    {
        if (msg->talkTo() == listener->id())
        {
            msgs.append(msg);
            old_msg_.removeOne(msg);
        }
    }
    return msgs;
}

void QQMsgCenter::removeListener(QQMsgListener *listener)
{
    listener_.removeOne(listener);
}


void QQMsgCenter::setMsgTip(MsgTip *msg_tip)
{
    msg_tip_ = msg_tip;
}

void QQMsgCenter::writeToSql(ShareQQMsgPtr msg)
{
    Q_UNUSED(msg)
}

QQMsgCenter::QQMsgCenter(MsgTip *msg_tip) :
    msg_tip_(msg_tip)
{
    qRegisterMetaType<FriendStatus>("FriendStatus");
    connect(this, SIGNAL(distributeMsgInMainThread(QQMsgListener*, ShareQQMsgPtr)), this, SLOT(distributeMsg(QQMsgListener*, ShareQQMsgPtr)));
}

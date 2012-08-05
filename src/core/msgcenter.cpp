#include "msgcenter.h"

#include <QMetaType>

#include "qqmsglistener.h"
#include "../msgtip.h"

void MsgCenter::pushMsg(ShareQQMsgPtr msg)
{
    undispatch_msg_.enqueue(msg);
    parse_done_smp_.release();
}

void MsgCenter::run()
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

        case QQMsg::kSess:
        case QQMsg::kFriend:
        case QQMsg::kGroup:
        {
            if (msg->type() == QQMsg::kFriend)
                emit friendChatMsgArrive(msg->talkTo());
            else if (msg->type() == QQMsg::kGroup)
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
                emit newUnProcessMsg(msg);
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

void MsgCenter::distributeMsg(QQMsgListener *listener, ShareQQMsgPtr msg)
{
    listener->showMsg(msg);
}

void MsgCenter::registerListener(QQMsgListener *listener)
{
    lock_.lock();
    QVector<ShareQQMsgPtr> old_msgs(getOldMsg(listener));
    listener->showOldMsg(old_msgs);
    lock_.unlock();
    listener_.append(listener);
}

QVector<ShareQQMsgPtr> MsgCenter::getOldMsg(QQMsgListener *listener)
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

void MsgCenter::removeListener(QQMsgListener *listener)
{
    listener_.removeOne(listener);
}


void MsgCenter::setMsgTip(MsgTip *msg_tip)
{
    msg_tip_ = msg_tip;
}

void MsgCenter::writeToSql(ShareQQMsgPtr msg)
{
    Q_UNUSED(msg)
}

MsgCenter::MsgCenter(MsgTip *msg_tip) :
    msg_tip_(msg_tip)
{
    qRegisterMetaType<FriendStatus>("FriendStatus");
    connect(this, SIGNAL(distributeMsgInMainThread(QQMsgListener*, ShareQQMsgPtr)), this, SLOT(distributeMsg(QQMsgListener*, ShareQQMsgPtr)));
}

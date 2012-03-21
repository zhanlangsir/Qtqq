#include "qqmsgcenter.h"
#include "qqmsg.h"
#include "qqmsgtip.h"
#include "qqmsglistener.h"

#include <QMutex>
#include <QWaitCondition>
#include <QSemaphore>
#include <QMetaType>

void QQMsgCenter::pushMsg(QQMsg *msg)
{
    undispatch_msg_.enqueue(msg);
    parse_done_smp_->release();
}

void QQMsgCenter::run()
{
    while (true)
    {
        parse_done_smp_->acquire();
        lock_->lock();

        QQMsg *msg = undispatch_msg_.dequeue();

        switch(msg->type())
        {
        case QQMsg::kBuddiesStatusChange:
        {
            QQStatusChangeMsg *status_changed_msg = static_cast<QQStatusChangeMsg*>(msg);
            emit buddiesStateChange(status_changed_msg->uin_, status_changed_msg->status_);
            break;
        }

        case QQMsg::kFriend:
        case QQMsg::kGroup:
        {
            bool is_msg_processed = false;
            QQMsgListener *listener;
            foreach(listener, listener_)
            {
                if (msg->talkTo() == listener->talkTo())
                {
                    is_msg_processed = true;
                    emit distributeMsgInMainThread(listener, msg);
                }
            }

            if (!is_msg_processed)
            {
                msg_tip_->pushMsg(msg);
                old_msg_.enqueue(msg);
            }
        }
            break;

        case QQMsg::kSystem:
            break;

        default:
            break;
        }
        lock_->unlock();

        writeToSql(msg);
    }
}

void QQMsgCenter::distributeMsg(QQMsgListener *listener, const QQMsg *msg)
{
    listener->showMsg(msg);
}

void QQMsgCenter::registerListener(QQMsgListener *listener)
{
    lock_->lock();
    listener->showOldMsg(getOldMsg(listener));
    lock_->unlock();
    listener_.append(listener);
}

QVector<QQMsg*> QQMsgCenter::getOldMsg(QQMsgListener *listener)
{
    QQMsg *msg = NULL;
    QVector<QQMsg*> msgs;
    foreach(msg, old_msg_)
    {
        if (msg->talkTo() == listener->talkTo())
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


void QQMsgCenter::setMsgTip(QQMsgTip *msg_tip)
{
    msg_tip_ = msg_tip;
}

void QQMsgCenter::writeToSql(QQMsg *msg)
{
}

QQMsgCenter::QQMsgCenter(QQMsgTip *msg_tip,
//                         QWaitCondition *wait_for_check_done,
 //                        QWaitCondition *wait_for_push_old_msg) : 
                           QMutex *lock,
                           QSemaphore *parse_done_smp):
    msg_tip_(msg_tip), lock_(lock), parse_done_smp_(parse_done_smp)
{
    qRegisterMetaType<FriendStatus>("FriendStatus");
    connect(this, SIGNAL(distributeMsgInMainThread(QQMsgListener*,const QQMsg*)), this, SLOT(distributeMsg(QQMsgListener*,const QQMsg*)));
}

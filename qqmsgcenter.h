/*
 * 获取消息可以通过buddiesStateChangeMsgArrive,groupChatMsgArrive等信号获取对应种类消息，也可以
 *通过注册为监听者获取id相关的消息，即监听者会获取跟自身id一直的消息,通过信号获取消息，消息不会
 *删除，而一旦监听者获取消息后，消息会被处理。不能再次使用，所以你应该一次性处理消息，可以保持
 *消息的值，但不能保存消息指针.
 *
 */

#pragma once

#include "types.h"
#include "qqmsg.h"

#include <QThread>
#include <QQueue>
#include <QLinkedList>
#include <QVector>
#include <QMutex>
#include <QSemaphore>

class QQMsgTip;
class QQMsgListener;
class QQGroupChatMsg;
class QQChatMsg;

class QQMsgCenter : public QThread
{
    Q_OBJECT
signals:
    void distributeMsgInMainThread(QQMsgListener *listener, ShareQQMsgPtr msg);
    void buddiesStateChangeMsgArrive(QString uin, FriendStatus state, ClientType client_type);
    void groupChatMsgArrive(QString id);
    void friendChatMsgArrive(QString id);


public:
       QQMsgCenter(QQMsgTip *msg_tip);

public:
       void registerListener(QQMsgListener *listener);
       void removeListener(QQMsgListener *listener);
       void setMsgTip(QQMsgTip *msg_tip);

public slots:
    void pushMsg(ShareQQMsgPtr msg);

protected:
    void run();

private slots:
    void distributeMsg(QQMsgListener *listener, ShareQQMsgPtr msg);

private:
    void writeToSql(ShareQQMsgPtr msg);
    QVector<ShareQQMsgPtr> getOldMsg(QQMsgListener *listener);

private:
    QQueue<ShareQQMsgPtr> undispatch_msg_;
    QQueue<ShareQQMsgPtr> old_msg_;
    QLinkedList<QQMsgListener*> listener_;
    QQMsgTip *msg_tip_;
    QMutex lock_;
    QSemaphore parse_done_smp_;
};

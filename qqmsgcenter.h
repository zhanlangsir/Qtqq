/*
 * 获取消息可以通过buddiesStateChangeMsgArrive,groupChatMsgArrive等信号获取对应种类消息，也可以
 *通过注册为监听者获取id相关的消息，即监听者会获取跟自身id一直的消息,通过信号获取消息，消息不会
 *删除，而一旦监听者获取消息后，消息会被处理。不能再次使用，所以你应该一次性处理消息，可以保持
 *消息的值，但不能保存消息指针.
 *
 */

#pragma once

#include "types.h"

#include <QThread>
#include <QQueue>
#include <QLinkedList>
#include <QVector>

class QQMsg;
class QQMsgTip;
class QMutex;
class QWaitCondition;
class QQMsgListener;
class QSemaphore;
class QQGroupChatMsg;
class QQChatMsg;

class QQMsgCenter : public QThread
{
    Q_OBJECT
signals:
    void distributeMsgInMainThread(QQMsgListener *listener, QQMsg *msg);
    void buddiesStateChangeMsgArrive(QString uin, FriendStatus state, ClientType client_type);
    void groupChatMsgArrive(const QQGroupChatMsg *msg);
    void friendChatMsgArrive(const QQChatMsg *msg);


public:
       QQMsgCenter(QQMsgTip *msg_tip, 
                   QMutex *lock,
                   QSemaphore *parse_done_smp);

public:
       void registerListener(QQMsgListener *listener);
       void removeListener(QQMsgListener *listener);
       void setMsgTip(QQMsgTip *msg_tip);

public slots:
    void pushMsg(QQMsg *msg);

protected:
    void run();

private slots:
    void distributeMsg(QQMsgListener *listener, QQMsg *msg);

private:
    void writeToSql(QQMsg *msg);
    QVector<QQMsg*> getOldMsg(QQMsgListener *listener);

private:
    QQueue<QQMsg*> undispatch_msg_;
    QQueue<QQMsg*> old_msg_;
    QLinkedList<QQMsgListener*> listener_;
    QQMsgTip *msg_tip_;
    //QWaitCondition *wait_for_check_done_, *wait_for_push_old_msg_;
    QMutex *lock_;
    QSemaphore *parse_done_smp_;
};

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

class QQMsgCenter : public QThread
{
    Q_OBJECT
signals:
    void distributeMsgInMainThread(QQMsgListener *listener, const QQMsg *msg);
    void buddiesStateChange(QString uin, FriendStatus state);

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
    void distributeMsg(QQMsgListener *listener, const QQMsg *msg);

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

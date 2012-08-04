#ifndef QTQQ_MSGCENTER_H
#define QTQQ_MSGCENTER_H

#include "types.h"
#include "qqmsg.h"

#include <QThread>
#include <QQueue>
#include <QLinkedList>
#include <QVector>
#include <QMutex>
#include <QSemaphore>

class MsgTip;
class QQMsgListener;
class QQGroupChatMsg;
class QQChatMsg;

class MsgCenter : public QThread
{
    Q_OBJECT
signals:
    void distributeMsgInMainThread(QQMsgListener *listener, ShareQQMsgPtr msg);
    void buddiesStateChangeMsgArrive(QString uin, FriendStatus state, ClientType client_type);
    void groupChatMsgArrive(QString id);
    void friendChatMsgArrive(QString id);
    void newUnProcessMsg(ShareQQMsgPtr msg);


public:
       MsgCenter(MsgTip *msg_tip);

public:
       void registerListener(QQMsgListener *listener);
       void removeListener(QQMsgListener *listener);
       void setMsgTip(MsgTip *msg_tip);

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
    MsgTip *msg_tip_;
    QMutex lock_;
    QSemaphore parse_done_smp_;
};

#endif //QTQQ_MSGCENTER_H

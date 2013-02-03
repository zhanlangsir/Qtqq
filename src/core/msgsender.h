#ifndef QTQQ_CORE_MSGSENDER_H
#define QTQQ_CORE_MSGSENDER_H

#include <QThread>
#include <QVector>
#include <QQueue>

#include "core/qqchatitem.h"
#include "request.h"
/*
class MsgSender : public QThread
{
    Q_OBJECT
signals:
    void sendDone(bool ok, QString msg);

public:
    MsgSender()
    {
        this->setTerminationEnabled(true);
    }

    void sendMsg(const QVector<QQChatItem> &items);
    void sendGroupMsg(const QVector<QQChatItem> &items);

public:
    void send(const Request &req);

private:
    QString groupMsgToJson(const QVector<QQChatItem> &items);
    QString msgToJson(const QVector<QQChatItem> &items);

protected:
    void run();

private:
    QQueue<Request> msgs_be_send_;
};
*/

#endif //QTQQ_CORE_MSGSENDER_H

#ifndef QTQQ_CORE_MSGSENDER_H
#define QTQQ_CORE_MSGSENDER_H

#include "request.h"

#include <QThread>
#include <QQueue>

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

public:
    void send(const Request &req);

protected:
    void run();

private:
    QQueue<Request> msgs_be_send_;
};

#endif //QTQQ_CORE_MSGSENDER_H

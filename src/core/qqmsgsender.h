#ifndef QTQQ_CORE_QQMSGSENDER_H
#define QTQQ_CORE_QQMSGSENDER_H

#include "request.h"

#include <QThread>
#include <QQueue>

class QQMsgSender : public QThread
{
    Q_OBJECT
signals:
    void sendDone(bool ok, QString msg);

public:
    QQMsgSender()
    {
        this->setTerminationEnabled(true);
    }

public:
    void send(const Request &msg);

protected:
    void run();

private:
    QQueue<Request> msgs_be_send_;
};

#endif //QTQQ_CORE_QQMSGSENDER_H

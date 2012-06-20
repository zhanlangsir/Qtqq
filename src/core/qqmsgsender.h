#pragma once

#include "request.h"

#include <QThread>
#include <QQueue>

class QQMsgSender : public QThread
{
public:
    QQMsgSender()
    {
        this->setTerminationEnabled(true);
    }

    ~QQMsgSender();

public:
    void send(const Request &msg);

protected:
    void run();

private:
    QQueue<Request> msgs_be_send_;
};

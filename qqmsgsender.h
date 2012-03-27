#pragma once

#include "request.h"

#include <QThread>
#include <QQueue>
#include <QSemaphore>

class QQMsgSender : public QThread
{
public:
    ~QQMsgSender();

public:
    void send(const Request &msg);

protected:
    void run();

private:
    QQueue<Request> msgs_be_send_;
    //QSemaphore msg_count_;
};

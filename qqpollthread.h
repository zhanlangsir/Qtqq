#pragma once

#include "types.h"
#include "request.h"
#include <QThread>
#include <QString>
#include <QTcpSocket>
#include <QSemaphore> 
#include <QQueue> 
class QQPollThread : public QThread
{
    Q_OBJECT

public:
    QQPollThread(QQueue<QByteArray> *message_queue,
                 QSemaphore *semaphore);

protected:
    void run();

private: 
   QTcpSocket *poll_fd_;
   QByteArray msg_;
   Request req_;
   QSemaphore *semaphore_;
   QQueue<QByteArray> *message_queue_;
};

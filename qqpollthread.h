#ifndef QTQQ_QQPOLLTHREAD_H
#define QTQQ_QQPOLLTHREAD_H

#include <QThread>
#include <QString>
#include <QSemaphore> 
#include <QQueue> 

#include "types.h"
#include "request.h"

class QQPollThread : public QThread
{
    Q_OBJECT
signals:
    void signalNewMsgArrive(QByteArray msg);
    
public:
    QQPollThread();

protected:
    void run();

private: 
   Request req_;
};

#endif //QTQQ_QQPOLLTHREAD_H

#ifndef QTQQ_CORE_QQPOLLTHREAD_H
#define QTQQ_CORE_QQPOLLTHREAD_H

#include <QThread>
#include <QString>

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

#endif //QTQQ_CORE_QQPOLLTHREAD_H

#ifndef QTQQ_CORE_POLLTHREAD_H
#define QTQQ_CORE_QQPOLLTHREAD_H

#include <QThread>
#include <QString>

#include "types.h"
#include "request.h"

class PollThread : public QThread
{
    Q_OBJECT
signals:
    void signalNewMsgArrive(QByteArray msg);
    
public:
    PollThread();

protected:
    void run();

private: 
   Request req_;
};

#endif //QTQQ_CORE_POLLTHREAD_H

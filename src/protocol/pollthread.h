#ifndef POLLTHREAD_H
#define POLLTHREAD_H

#include <QThread>
#include <QString>

#include "core/request.h"

namespace Protocol
{
	class PollThread;
}

class Protocol::PollThread : public QThread
{
    Q_OBJECT
signals:
    void newMsgArrive(QByteArray msg);
    
public:
    PollThread(QObject *parent = NULL);

	void stop();

protected:
    void run();

private: 
   Request req_;
   bool is_stop_;
};

#endif //POLLTHREAD_H

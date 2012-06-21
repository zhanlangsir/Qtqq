#ifndef QTQQ_CORE_PARSETHREAD_H
#define QTQQ_CORE_PARSETHREAD_H

#include "qqmsg.h"
#include "include/json.h"

#include <QThread>
#include <QString>
#include <QSemaphore>
#include <QQueue>
#include <QMutex>

class ParseThread : public QThread
{
    Q_OBJECT
public:
    ParseThread();

public slots:
    void pushRawMsg(QByteArray msg);

signals:
    void parseDone(ShareQQMsgPtr msg);

protected:
    void run();

private:
    QQMsg *createMsg(QString type, const Json::Value result);

private:
   QQueue<QByteArray> message_queue_;
   QMutex lock_;
};

#endif //QTQQ_CORE_PARSETHREAD_H

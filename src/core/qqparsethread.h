#ifndef QTQQ_CORE_QQPARSETHREAD_H
#define QTQQ_CORE_QQPARSETHREAD_H

#include "qqmsg.h"
#include "include/json.h"

#include <QThread>
#include <QString>
#include <QSemaphore>
#include <QQueue>
#include <QMutex>

class QQParseThread : public QThread
{
    Q_OBJECT
public:
    QQParseThread();

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

#endif //QTQQ_CORE_QQPARSETHREAD_H

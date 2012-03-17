#pragma once

#include "qqmsg.h"
#include "include/json/json.h"

#include <QThread>
#include <QString>
#include <QSemaphore>
#include <QQueue>
#include <QMutex>

class QQParseThread : public QThread
{
    Q_OBJECT

public:
    QQParseThread(QQueue<QByteArray> *message_queue, QSemaphore *poll_done_smp, QSemaphore *parse_done_smp);

signals:
    void parseDone(QQMsg *msg);

protected:
    void run();

private:
    QQMsg *createMsg(QString type, const Json::Value result);

private:
   QSemaphore *poll_done_smp_;
   QSemaphore *parse_done_smp_;
   QQueue<QByteArray> *message_queue_;
};

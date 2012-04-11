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

public slots:
    void pushRawMsg(QByteArray msg);

signals:
    void parseDone(QQMsg *msg);

protected:
    void run();

private:
    QQMsg *createMsg(QString type, const Json::Value result);

private:
   QQueue<QByteArray> message_queue_;
   QMutex lock_;
};

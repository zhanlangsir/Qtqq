#ifndef QTQQ_CORE_PARSETHREAD_H
#define QTQQ_CORE_PARSETHREAD_H

#include "qqmsg.h"
#include <json/json.h>

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
    QQMsg *createFriendMsg(const Json::Value &result) const;
    QQMsg *createGroupMsg(const Json::Value &result) const;
    QQMsg *createSessMsg(const Json::Value &result) const;
    QQMsg *createBuddiesStatusChangeMsg(const Json::Value &result) const;
    QQMsg *createSystemGroupMsg(const Json::Value &result) const;
    QQMsg *createSystemMsg(const Json::Value &result) const;

    bool isChatContentEmpty(QString content) const;

    void sortByTime(QVector<QQMsg*> &be_sorting_msg) const;

private:
   QQueue<QByteArray> message_queue_;
   QMutex lock_;

   Json::Value pre_msg_;
};

#endif //QTQQ_CORE_PARSETHREAD_H

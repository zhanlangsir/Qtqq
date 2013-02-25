#ifndef SENDMSG_JOB_H
#define SENDMSG_JOB_H

#include <QVector>
#include <QHttp>

#include "job_base.h"
#include "core/qqchatitem.h"

class Talkable;
class SendImgJob;

class SendMsgJob : public __JobBase
{
    Q_OBJECT
public:
    SendMsgJob(Talkable *_for, const QVector<QQChatItem> &msgs, JobType type = JT_SendMsg);

    virtual void run();

protected:
    void sendMsg();

protected:
    QVector<QQChatItem> msgs_;

private slots:
    void requestDone(bool error);

private:
    QHttp http_;
};

class SendFriendMsgJob : public SendMsgJob
{
    Q_OBJECT
public:
    SendFriendMsgJob(Talkable *_for, const QVector<QQChatItem> &msgs) : 
        SendMsgJob(_for, msgs),
        finish_imgjobs_(0)
    {
    }

    virtual void run();

private slots:
    void onImgSendDone(__JobBase *job, bool error);

private:
    QVector<QString> jobs_;
    int finish_imgjobs_;
};

class SendGroupMsgJob : public SendMsgJob
{
    Q_OBJECT
public:
    SendGroupMsgJob(Talkable *_for, const QVector<QQChatItem> &msgs) :
        SendMsgJob(_for, msgs),
        finish_imgjobs_(0)
    {
    }

    virtual void run();

private slots:
    void onImgSendDone(__JobBase *job, bool error);

private:
    QVector<QString> jobs_;
    int finish_imgjobs_;
};

#endif //SENDMSG_JOB_H

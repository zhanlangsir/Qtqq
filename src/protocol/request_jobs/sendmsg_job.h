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
    virtual QString getPath() const = 0;
    virtual QByteArray getData() const = 0;

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
    virtual QString getPath() const;
    virtual QByteArray getData() const;

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
    virtual QString getPath() const;
    virtual QByteArray getData() const;

private slots:
    void onImgSendDone(__JobBase *job, bool error);

private:
    QVector<QString> jobs_;
    int finish_imgjobs_;
};

class SendSessMsgJob : public SendMsgJob
{
public:
    SendSessMsgJob(Talkable *_for, Group *group, const QVector<QQChatItem> &msgs) :
        SendMsgJob(_for, msgs),
        group_(group)
    {
    }

    virtual QString getPath() const;
    virtual QByteArray getData() const;

private:
    Group *group_;
};

#endif //SENDMSG_JOB_H

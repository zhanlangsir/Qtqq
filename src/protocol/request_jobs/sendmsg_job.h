#ifndef SENDMSG_JOB_H
#define SENDMSG_JOB_H

#include <QHttp>

#include "job_base.h"

class Talkable;

class SendMsgJob : public __JobBase
{
    Q_OBJECT
public:
    SendMsgJob(Talkable *_for, const QString &data, JobType type = JT_SendMsg);

    virtual void run();

private slots:
    void requestDone(bool error);

private:
    QHttp http_;
    QString data_;
};

#endif //SENDMSG_JOB_H

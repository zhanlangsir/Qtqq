#ifndef SENDIMG_JOB_H
#define SENDIMG_JOB_H

#include <QHttp>

#include "protocol/request_jobs/job_base.h"

class Talkable;

class SendImgJob : public __JobBase
{
    Q_OBJECT
public:
    SendImgJob(Talkable *_for, const QString &file_path, const QByteArray &data, JobType type = JT_SendImg);

    virtual void run();

private slots:
    void requestDone(bool error);

private:
    QHttp http_;
    QString file_path_;
    QByteArray data_;
};

#endif //SENDIMG_JOB_H

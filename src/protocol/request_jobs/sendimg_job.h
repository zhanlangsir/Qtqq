#ifndef SENDIMG_JOB_H
#define SENDIMG_JOB_H

#include <QHttp>

#include "protocol/request_jobs/job_base.h"

class Talkable;

class SendImgJob : public __JobBase
{
    Q_OBJECT
public:
    enum SendImgType { kOffpic, kGroupImg };
    SendImgJob(const QString &file_path, const QByteArray &data, SendImgType sendimg_type, JobType type = JT_SendImg);

    QString filePath() const
    { return file_path_; }

    virtual void run();

private slots:
    void requestDone(bool error);

private:
    QHttp http_;

    SendImgType sendimg_type_;
    QString file_path_;
    QByteArray data_;
};

#endif //SENDIMG_JOB_H

#ifndef SENDFILE_JOB_H
#define SENDFILE_JOB_H

#include <QHttp>

#include "protocol/request_jobs/job_base.h"

class Talkable;

class SendFileJob : public __JobBase
{
    Q_OBJECT
signals:
    void sigJobDone(__JobBase *job, bool err);
    void sendFileProgress(QString file, int recived_byte, int total_byte);

public:
    enum SendFileJobType { kOffFile, kFile };
    SendFileJob(const QString &to_id, const QString &file_path, const QByteArray &data, SendFileJobType filejob_type, JobType type = JT_SendFile);


    QString filePath() const
    { return file_path_; }

    virtual void run();

    void stop();

private slots:
    void requestDone(bool error);
    void onDataSendProgress(int done, int total);

private:
    void notifyServer(QString file_path);
    QString getFilePath(const QByteArray &data);

private:
    QHttp http_;

    SendFileJobType filejob_type_;

    QString to_id_;
    QString file_path_;
    QString file_;
    QByteArray data_;
};

#endif //SENDFILE_JOB_H

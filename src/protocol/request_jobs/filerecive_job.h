#ifndef FILERECIVE_JOB_H
#define FILERECIVE_JOB_H

#include <QHttp>
#include <QString>

#include "protocol/request_jobs/job_base.h"

class QFile;

class FileReciveJob : public __JobBase
{
    Q_OBJECT
signals:
    void sigJobDone(__JobBase *job, bool err);
    void fileTransferProgress(int session_id, int recived_byte, int total_byte);

public:
    FileReciveJob(int session_id, QString file, QString to, JobType type = JT_FileRecive);

    int sessionId() const
    { return session_id_; }
    virtual void run();
    void stop();

private slots:
    void requestLocationDone(bool err);
    void requestFileDone(bool err);
    void onDataReadProgress(int done, int total);

private:
    QHttp http_;
    QFile *save_to_;
    int session_id_;
    QString file_;
    QString to_;

    static QString save_dir_;
};
#endif //FILERECIVE_JOB_H

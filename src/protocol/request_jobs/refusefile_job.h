#ifndef REFUSEFILE_JOB_H
#define REFUSEFILE_JOB_H

#include <QHttp>

#include "protocol/request_jobs/job_base.h"

class RefuseFileJob : public __JobBase
{
    Q_OBJECT
public:
    RefuseFileJob(const QString &to_id, int session_id, JobType type = JT_RefuseFile);

    virtual void run();

private slots:
    void requestDone(bool error);

private:
    QHttp http_;

    QString to_id_;
    int session_id_;
};
#endif //REFUSEFILE_JOB_H

#ifndef GROUPMEMBERLISTJOB_H
#define GROUPMEMBERLISTJOB_H

#include <QHttp>

#include "protocol/request_jobs/job_base.h"

class GroupMemberListJob : public __JobBase
{
    Q_OBJECT
signals:
    void sigJobDone(__JobBase *job, bool err);

public:
    GroupMemberListJob(Group *group, JobType type = JT_GroupMemberList);

    virtual void run();

private slots:
    void requestDone(bool err);

private:
    QHttp http_;
};

#endif //GROUPMEMBERLISTJOB_H

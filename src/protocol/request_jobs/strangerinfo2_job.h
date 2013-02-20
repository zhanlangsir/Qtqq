#ifndef STRANGERINFO2_JOB_H
#define STRANGERINFO2_JOB_H

#include <QHttp>

#include "protocol/request_jobs/job_base.h"

class StrangerInfo2Job : public __JobBase
{
	Q_OBJECT
signals:
	void sigJobDone(__JobBase *job, bool error);

public:
	StrangerInfo2Job(Talkable *job_for, QString gid, QString code, JobType type = JT_StrangerInfo2);

	virtual void run();

private slots:
	void requestDone(bool error);

private:
	QHttp http_;

    QString gid_;
    QString code_;
};

#endif //STRANGERINFO2_JOB_H

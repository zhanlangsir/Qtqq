#ifndef FRIENDINFO2_JOB_H
#define FRIENDINFO2_JOB_H

#include <QHttp>

#include "qq_protocol/request_jobs/job_base.h"

class FriendInfo2Job : public __JobBase
{
	Q_OBJECT
signals:
	void sigJobDone(__JobBase *job, bool error);

public:
	FriendInfo2Job(QString id, RequestCallbackBase *callback, JobType type = JT_FriendInfo2);	

	virtual void run();

private slots:
	void requestDone(bool error);

private:
	QHttp http_;
};

#endif //FRIENDINFO2_JOB_H

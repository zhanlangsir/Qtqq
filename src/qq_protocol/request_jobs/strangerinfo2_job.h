#ifndef STRANGERINFO2_JOB_H
#define STRANGERINFO2_JOB_H

#include <QHttp>

#include "qq_protocol/request_jobs/job_base.h"

class StrangerInfo2Job : public __JobBase
{
	Q_OBJECT
signals:
	void sigJobDone(__JobBase *job, bool error);

public:
	StrangerInfo2Job(QString id, QString gid, RequestCallbackBase *callback, JobType type = JT_StrangerInfo2);

	virtual void run();

private slots:
	void requestDone(bool error);

private:
	QString gid_;
	QHttp http_;
};

#endif //STRANGERINFO2_JOB_H

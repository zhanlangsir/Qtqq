#ifndef ICON_JOB_H
#define ICON_JOB_H

#include <QHttp>

#include "job_base.h"

class Talkable;
class RequestCallbackBase;

class IconJob : public __JobBase
{
	Q_OBJECT
public:
	IconJob(QString requester_id, QString icon_id, int request_type, RequestCallbackBase *callback = NULL, JobType type = JT_Icon);

	static int getRequestType(Talkable *talkable);

	virtual void run();

private slots:
	void requestDone(bool error);

private:
	int request_type_;
	QString icon_id_;

	QHttp http_;
};

#endif //ICON_JOB_H

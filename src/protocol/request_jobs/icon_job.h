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
	IconJob(Talkable *_for, JobType type = JT_Icon);

	virtual void run();

private slots:
	void requestDone(bool error);

private:
	int getRequestType(Talkable *job_for);
    QString getRequestId(Talkable *job_for);

    void triggerEvent(const QByteArray &data);

private:
	QHttp http_;
};

#endif //ICON_JOB_H

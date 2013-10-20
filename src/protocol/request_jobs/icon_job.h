#ifndef ICON_JOB_H
#define ICON_JOB_H

#include "job_base.h"

class Talkable;
class RequestCallbackBase;

class IconJob : public __JobBase
{
	Q_OBJECT
public:
	IconJob(Talkable *_for, JobType type = JT_Icon);
    void setGid(QString gid)
    {
        gid_ = gid;
    }

	virtual void run();

	static int getRequestType(Talkable *job_for);
    static QString getRequestId(Talkable *job_for);

private slots:
	void requestDone();

private:
    void triggerEvent(const QByteArray &data);

private:
    Talkable::TalkableType t_type_;
    Talkable *talkable_;

    QString gid_;
};

#endif //ICON_JOB_H

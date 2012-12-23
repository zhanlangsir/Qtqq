#ifndef JOB_BASE_H
#define JOB_BASE_H

#include <QObject>

#include "utils/request_callback.h"

enum JobType
{
	JT_Icon,
	JT_FriendInfo2,
	JT_StrangerInfo2
};

class __JobBase : public QObject
{
	Q_OBJECT
signals:
	void sigJobDone(__JobBase *job, bool error);

public:
	__JobBase(QString id, JobType type, RequestCallbackBase* callback = NULL) : 
		type_(type),
		id_(id),
		callback_(callback)
	{}

	virtual ~__JobBase()
	{
		if ( callback_ )
			delete callback_;
		callback_ = NULL;
	}

	JobType type() const
	{
		return type_;
	}
	QString id() const
	{
		return id_;
	}

	virtual void run() = 0;
	
protected:
	JobType type_;

	QString id_;
	RequestCallbackBase *callback_;
};

#endif //JOB_BASE_H

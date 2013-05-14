#ifndef JOB_BASE_H
#define JOB_BASE_H

#include <QObject>

#include "core/talkable.h"

enum JobType
{
	JT_Icon,
	JT_FriendInfo2,
	JT_StrangerInfo2,
    JT_GroupMemberList,
    JT_LoadImg,
    JT_SendImg,
    JT_SendFile,
    JT_SendMsg,
    JT_FileRecive,
    JT_RefuseFile
};

class __JobBase : public QObject
{
	Q_OBJECT
signals:
	void sigJobDone(__JobBase *job, bool error);

public:
	__JobBase(QString id, JobType type) : 
		type_(type),
        id_(id)
	{
    }

	virtual ~__JobBase()
    {
    }

	JobType type() const
	{
		return type_;
	}

    QString jobFor() const
    {
        return id_;
    }

    QString requesterId()
    {
        return id_;
    }

	virtual void run() = 0;
	
protected:
	JobType type_;
    QString id_;
};

#endif //JOB_BASE_H

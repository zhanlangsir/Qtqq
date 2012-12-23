#include "qq_protocol.h"

#include <QDebug>

#include "roster/roster.h"
#include "core/talkable.h"
#include "storage/storage.h"
#include "qq_protocol/request_jobs/icon_job.h"
#include "qq_protocol/request_jobs/strangerinfo2_job.h"
#include "qq_protocol/request_jobs/friendinfo2_job.h"
#include "qq_protocol/pollthread.h"

Protocol::QQProtocol* Protocol::QQProtocol::instance_ = NULL;

Protocol::QQProtocol::QQProtocol() :
	poll_thread_(new Protocol::PollThread(this))
{
	connect(poll_thread_, SIGNAL(newMsgArrive(QByteArray)), this, SIGNAL(newQQMsg(QByteArray)));
}

Protocol::QQProtocol::~QQProtocol()
{
	stop();
}

void Protocol::QQProtocol::run()
{
	poll_thread_->start();
}


void Protocol::QQProtocol::stop()
{
	poll_thread_->stop();
}


void Protocol::QQProtocol::requestIconFor(QString requester_id, RequestCallbackBase* callback)
{
	if ( !Storage::instance()->icon(requester_id).isNull() )
	{
		callback->callback();

		delete callback;
		callback = NULL;

		return;
	}

	Roster *roster = Roster::instance();
	Talkable* talkable = roster->talkable(requester_id);
	int request_type = IconJob::getRequestType(talkable);

	IconJob* job = NULL;
	QString icon_id;

	if ( !talkable || talkable->type() == Talkable::kContact )
	{
		icon_id = requester_id;
	}
	else
	{
		icon_id = talkable->gcode();
	}

	job = new IconJob(requester_id, icon_id, request_type, callback);
	connect(job, SIGNAL(sigJobDone(__JobBase*, bool)), this, SLOT(slotJobDone(__JobBase*, bool)));
	job->run();
}

void Protocol::QQProtocol::requestSingleLongNick(QString id, RequestCallbackBase* callback)
{

}

void Protocol::QQProtocol::requestContactInfo(QString id, RequestCallbackBase* callback)
{

}

void Protocol::QQProtocol::requestFriendInfo2(QString id, RequestCallbackBase *callback)
{
	FriendInfo2Job *job = new FriendInfo2Job(id, callback);
	connect(job, SIGNAL(sigJobDone(__JobBase*, bool)), this, SLOT(slotJobDone(__JobBase*, bool)));
	job->run();
}

void Protocol::QQProtocol::requestStrangerInfo2(QString id, QString gid, RequestCallbackBase *callback)
{
	requesting_.insert(JT_StrangerInfo2, id);
	StrangerInfo2Job *job = new StrangerInfo2Job(id, gid, callback);
	connect(job, SIGNAL(sigJobDone(__JobBase*, bool)), this, SLOT(slotJobDone(__JobBase*, bool)));
	job->run();
}

void Protocol::QQProtocol::slotJobDone(__JobBase* job, bool error)
{
	if ( !error )
	{
		requesting_.values(job->type()).removeOne(job->id());

		delete job;
		job = NULL;
	}
	else
		qDebug() << "run job failed!" << endl;
}

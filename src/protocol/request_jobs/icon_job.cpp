#include "icon_job.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDebug>

#include "core/talkable.h"
#include "core/captchainfo.h"
#include "protocol/qq_protocol.h"
#include "protocol/event.h"
#include "protocol/event_center.h"

IconJob::IconJob(Talkable *job_for, JobType type) : 
	__JobBase(job_for->id(), type),
    t_type_(job_for->type()),
    talkable_(job_for)
{
}

int IconJob::getRequestType(Talkable *job_for)
{
    if ( job_for->type() == Talkable::kContact || job_for->type() == Talkable::kStranger || job_for->type() == Talkable::kSessStranger )
		return 1;
	else if ( job_for->type() == Talkable::kGroup )
		return 4;

	return -1;
}

void IconJob::triggerEvent(const QByteArray &data)
{
    Protocol::Event *event = NULL;
    if ( t_type_ == Talkable::kStranger )
    {
        event = Protocol::EventCenter::instance()->createStrangerAvatarUpdateEvent(id_, t_type_, data);
    }
    else if ( t_type_ == Talkable::kSessStranger )
    {
        event = Protocol::EventCenter::instance()->createGroupMemberAvatarUpdateEvent(id_, gid_, t_type_, data);
    }
    else
    {
        event = Protocol::EventCenter::instance()->createAvatarUpdateEvent(id_, t_type_, data);
    }
    Protocol::EventCenter::instance()->triggerEvent(event);
}

void IconJob::requestDone()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
	if ( !reply->error() )
	{
		QByteArray data = reply->readAll();
		reply->close();
        reply->deleteLater();

        triggerEvent(data);
	}
	else
	{
		qDebug() << "request icon for " << id_ << " failed! " << endl;
		qDebug() << "error: " << reply->error() << endl;
	}

	emit sigJobDone(this, reply->error());
}

QString IconJob::getRequestId(Talkable *job_for)
{
    if ( job_for->type() == Talkable::kGroup )
    {
        return static_cast<Group *>(job_for)->gcode(); 
    }
    else if ( job_for->type() == Talkable::kContact || job_for->type() == Talkable::kStranger || job_for->type() == Talkable::kSessStranger )
    {
        return job_for->id(); 
    }
}

void IconJob::run()
{
	QString request_path = "http://face1.qun.qq.com/cgi/svr/face/getface?cache=0&type=%1&fid=0&uin=%2&vfwebqq=%3";
    QNetworkRequest request(request_path.arg(getRequestType(talkable_)).arg(getRequestId(talkable_)).arg(CaptchaInfo::instance()->vfwebqq()));
    request.setRawHeader("Host", "face1.qun.qq.com");
	request.setRawHeader("Referer", "http://web.qq.com");
	request.setRawHeader("Cookie", CaptchaInfo::instance()->cookie().toLatin1());
	
	//http_.request(header);
    QNetworkReply *reply = Protocol::QQProtocol::instance()->networkMgr()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(requestDone()));
}

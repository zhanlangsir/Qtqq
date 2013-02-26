#include "icon_job.h"

#include <QHttpRequestHeader>
#include <QDebug>

#include "core/talkable.h"
#include "core/captchainfo.h"
#include "protocol/event.h"
#include "protocol/event_center.h"

IconJob::IconJob(Talkable *_for, JobType type) : 
	__JobBase(_for, type),
	http_(this)
{
	connect(&http_, SIGNAL(done(bool)), this, SLOT(requestDone(bool)));
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
    if ( for_->type() == Talkable::kStranger || for_->type() == Talkable::kSessStranger )
    {
        event = Protocol::EventCenter::instance()->createStrangerAvatarUpdateEvent(for_, data);
    }
    else
    {
        event = Protocol::EventCenter::instance()->createAvatarUpdateEvent(for_, data);
    }
    Protocol::EventCenter::instance()->triggerEvent(event);
}

void IconJob::requestDone(bool error)
{
	if ( !error )
	{
		QByteArray data = http_.readAll();
		http_.close();

        triggerEvent(data);
	}
	else
	{
		qDebug() << "request icon for " << for_->id() << " failed! " << endl;
		qDebug() << "error: " << http_.errorString() << endl;
	}

	http_.disconnect(this);
	emit sigJobDone(this, error);
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
	QHttpRequestHeader header;
	http_.setHost("face1.qun.qq.com");
	QString request_path = "/cgi/svr/face/getface?cache=0&type=%1&fid=0&uin=%2&vfwebqq=%3";
	header.setRequest("GET", request_path.arg(getRequestType(for_)).arg(getRequestId(for_)).arg(CaptchaInfo::instance()->vfwebqq()));
	header.addValue("Host", "face1.qun.qq.com");
	header.addValue("Referer", "http://web.qq.com");
	header.addValue("Cookie", CaptchaInfo::instance()->cookie());
	
	http_.request(header);
}

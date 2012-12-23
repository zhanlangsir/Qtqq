#include "icon_job.h"

#include <QHttpRequestHeader>
#include <QDebug>

#include "core/talkable.h"
#include "utils/request_callback.h"
#include "core/captchainfo.h"

IconJob::IconJob(QString requester_id, QString icon_id, int request_type, RequestCallbackBase *callback, JobType type) : 
	__JobBase(requester_id, type, callback),
	request_type_(request_type),
	icon_id_(icon_id),
	http_(this)
{
	connect(&http_, SIGNAL(done(bool)), this, SLOT(requestDone(bool)));
}

int IconJob::getRequestType(Talkable *talkable)
{
	if ( !talkable )
	{
		int stranger = 1;
		return stranger;
	}

	if ( talkable->type() == Talkable::kContact )	
		return 1;
	else if ( talkable->type() == Talkable::kGroup )
		return 4;

	return -1;
}

void IconJob::requestDone(bool error)
{
	if ( !error )
	{
		QByteArray data = http_.readAll();
		Storage::instance()->addIcon(id_, data);

		http_.close();

		callback_->callback();
	}
	else
	{
		qDebug() << "request icon for " << id_ << " failed! " << endl;
		qDebug() << "error: " << http_.errorString() << endl;
	}

	http_.disconnect(this);
	emit sigJobDone(this, error);
}


void IconJob::run()
{
	QHttpRequestHeader header;
	http_.setHost("face1.qun.qq.com");
	QString request_path = "/cgi/svr/face/getface?cache=0&type=%1&fid=0&uin=%2&vfwebqq=%3";
	header.setRequest("GET", request_path.arg(request_type_).arg(icon_id_).arg(CaptchaInfo::instance()->vfwebqq()));
	header.addValue("Host", "face1.qun.qq.com");
	header.addValue("Referer", "http://web.qq.com");
	header.addValue("Cookie", CaptchaInfo::instance()->cookie());
	
	http_.request(header);
}



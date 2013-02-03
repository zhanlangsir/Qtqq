#include "strangerinfo2_job.h"

#include <QHttpRequestHeader>
#include <QDateTime>
#include <QDebug>

#include "core/captchainfo.h"
#include "strangermanager/stranger_manager.h"

StrangerInfo2Job::StrangerInfo2Job(Talkable *job_for, JobType type) :
	__JobBase(job_for, type),
	http_(this)
{
	connect(&http_, SIGNAL(done(bool)), this, SLOT(requestDone(bool)));
}

void StrangerInfo2Job::run()
{

    QString get_stranger_info_url = "/api/get_stranger_info2?tuin=" + for_->id() + "&verifysession=&gid=0&code=group_request_join-"+((Contact *)for_)->group()->id() +"&vfwebqq=" +
            CaptchaInfo::instance()->vfwebqq() + "&t=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());

	QHttpRequestHeader header;
	http_.setHost("s.web2.qq.com");
	header.setRequest("GET", get_stranger_info_url);
	header.addValue("Host", "s.web2.qq.com");
	header.addValue("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001&callback=1&id=1");
	header.addValue("Connection", "keep-live");
	header.addValue("Content-Type","utf-8");
	header.addValue("Cookie", CaptchaInfo::instance()->cookie());

	http_.request(header);
}

void StrangerInfo2Job::requestDone(bool error)
{
	if ( !error )
	{
		QByteArray data = http_.readAll();
		http_.close();

		StrangerManager::instance()->parseStranger(data);
	}
	else
	{
		qDebug() << "request stranger for " << for_->id() << " failed! " << endl;
		qDebug() << "error: " << http_.errorString() << endl;
	}

	http_.disconnect(this);
	emit sigJobDone(this, error);
}

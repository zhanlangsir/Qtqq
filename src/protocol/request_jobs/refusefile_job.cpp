#include "refusefile_job.h"

#include <QDebug>
#include <QDateTime>

#include "core/captchainfo.h"

RefuseFileJob::RefuseFileJob(const QString &to_id, int session_id, JobType type) : 
	__JobBase(NULL, type),
	http_(this),
    to_id_(to_id),
    session_id_(session_id)
{
	connect(&http_, SIGNAL(done(bool)), this, SLOT(requestDone(bool)));
}

void RefuseFileJob::requestDone(bool error)
{
	if ( !error )
	{
		QByteArray data = http_.readAll();
		http_.close();

        qDebug() << "Refuse file done, data:\n" << data << endl;

        /*
        Protocol::Event *event = Protocol::EventCenter::instance()->createImgSendDoneEvent(for_, success, file_path_, img_id);
        Protocol::EventCenter::instance()->triggerEvent(event);
        */
	}
	else
	{
		qDebug() << "Refuse file for: " << to_id_ << "failed" << endl;
		qDebug() << "error: " << http_.errorString() << endl;
	}

	http_.disconnect(this);
	emit sigJobDone(this, error);
}

void RefuseFileJob::run()
{
    QString send_url = "/channel/refuse_file2?to=%1&lcid=%2&clientid=5412354841&psessionid=%3&t=%4";
    QString host = "d.web2.qq.com";

    QHttpRequestHeader header;
    header.setRequest("GET", send_url.arg(to_id_).arg(session_id_).arg(CaptchaInfo::instance()->psessionid()).arg(QDateTime::currentMSecsSinceEpoch()));
    header.addValue("Host", host);
    header.addValue("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002&callback=1&id=2");
    header.addValue("Cookie", CaptchaInfo::instance()->cookie());

    http_.setHost(host);
    http_.request(header);
}

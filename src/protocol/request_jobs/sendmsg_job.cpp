#include "sendmsg_job.h"

#include <QHttpRequestHeader>
#include <QDebug>

#include "core/talkable.h"
#include "core/captchainfo.h"
#include "protocol/event.h"
#include "protocol/event_center.h"

SendMsgJob::SendMsgJob(Talkable *_for, const QString &data, JobType type) : 
	__JobBase(_for, type),
	http_(this),
    data_(data)
{
	connect(&http_, SIGNAL(done(bool)), this, SLOT(requestDone(bool)));
}

void SendMsgJob::requestDone(bool error)
{
	if ( !error )
	{
		QByteArray data = http_.readAll();
		http_.close();

        Protocol::Event *event = Protocol::EventCenter::instance()->createMsgSendDoneEvent(for_, data);
        Protocol::EventCenter::instance()->triggerEvent(event);
	}
	else
	{
		qDebug() << "send msg for " << for_->id() << " " << for_->name() << " failed! " << endl;
		qDebug() << "error: " << http_.errorString() << endl;
	}

	http_.disconnect(this);
	emit sigJobDone(this, error);
}

void SendMsgJob::run()
{
    QString send_url;
    if ( for_->type() == Talkable::kContact )
        send_url = "/channel/send_buddy_msg2";
    else if ( for_->type() == Talkable::kGroup )
        send_url =  "/channel/send_qun_msg2";

    QHttpRequestHeader header;

    header.setRequest("POST", send_url);
    header.addValue("Host", "d.web2.qq.com");
    header.addValue("Cookie", CaptchaInfo::instance()->cookie());
    header.addValue("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002");
    header.addValue("Content-Length", QString::number(data_.length()));
    header.addValue("Content-Type", "application/x-www-form-urlencoded");

    http_.setHost("d.web2.qq.com");
    http_.request(header, data_.toAscii());
}

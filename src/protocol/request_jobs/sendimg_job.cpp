#include "sendimg_job.h"

#include <QHttpRequestHeader>
#include <QDateTime>
#include <QDebug>

#include "core/talkable.h"
#include "core/captchainfo.h"
#include "protocol/event.h"
#include "protocol/qq_protocol.h"
#include "protocol/event_center.h"
#include "protocol/imgsender.h"

SendImgJob::SendImgJob(Talkable *_for, const QString &file_path, const QByteArray &data, JobType type) : 
	__JobBase(_for, type),
	http_(this),
    file_path_(file_path),
    data_(data)
{
	connect(&http_, SIGNAL(done(bool)), this, SLOT(requestDone(bool)));
}

void SendImgJob::requestDone(bool error)
{
	if ( !error )
	{
		QByteArray data = http_.readAll();
		http_.close();

        qDebug() << "img request done, data:\n" << data << endl;
        Protocol::ImgSender *sender = Protocol::QQProtocol::instance()->imgSender();
        bool success = false;
        QString img_id;
        if ( for_->type() == Talkable::kContact )
            success = sender->parseMsgResult(data, img_id);
        else
            success = sender->parseGroupMsgResult(data, img_id);

        Protocol::Event *event = Protocol::EventCenter::instance()->createImgSendDoneEvent(for_, success, file_path_, img_id);
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

void SendImgJob::run()
{
    QString send_url;
    QString host;
    if ( for_->type() == Talkable::kContact )
    {
        send_url = "/ftn_access/upload_offline_pic?time=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
;
        host = "weboffline.ftn.qq.com";
    }
    else if ( for_->type() == Talkable::kGroup )
    {
        send_url = "/cgi-bin/cface_upload?time=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
;
        host = "up.web2.qq.com";
    }

    QHttpRequestHeader header;

    header.setRequest("POST", send_url);
    header.addValue("Host", host);
    header.addValue("Content-Length", QString::number(data_.length()));
    header.addValue("Cache-Control", "max-age=0");
    header.addValue("Origin", "http://web.qq.com");
    header.addValue("Content-Type", "multipart/form-data; boundary="+Protocol::QQProtocol::instance()->imgSender()->boundary());
    header.addValue("Referer", "http://web.qq.com/");
    header.addValue("Cookie", CaptchaInfo::instance()->cookie());

    http_.setHost(host);
    http_.request(header, data_);
}

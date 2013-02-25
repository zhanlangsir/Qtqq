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

SendImgJob::SendImgJob(const QString &file_path, const QByteArray &data, SendImgType sendimg_type, JobType type) : 
	__JobBase(NULL, type),
	http_(this),
    sendimg_type_(sendimg_type),
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

        qDebug() << "Image request done, data:\n" << data << endl;
        Protocol::ImgSender *sender = Protocol::QQProtocol::instance()->imgSender();
        bool success = false;
        QString img_id;


        if ( sendimg_type_ == kOffpic )
        {
            success = sender->parseMsgResult(file_path_, data);
        }
        else if ( sendimg_type_ == kGroupImg )
        {
            success = sender->parseGroupMsgResult(file_path_, data);
        }
        //Protocol::Event *event = Protocol::EventCenter::instance()->createImgSendDoneEvent(for_, success, file_path_, img_id);
        //Protocol::EventCenter::instance()->triggerEvent(event);
    }
	else
	{
		qDebug() << "Send image " << file_path_ << " " << " failed! " << endl;
		qDebug() << "error: " << http_.errorString() << endl;
	}

	http_.disconnect(this);
	emit sigJobDone(this, error);
}

void SendImgJob::run()
{
    QString send_url;
    QString host;
    if (  sendimg_type_ == kOffpic )
    {
        send_url = "/ftn_access/upload_offline_pic?time=" + QString::number(QDateTime::currentMSecsSinceEpoch());
;
        host = "weboffline.ftn.qq.com";
    }
    else if ( sendimg_type_ == kGroupImg )
    {
        send_url = "/cgi-bin/cface_upload?time=" + QString::number(QDateTime::currentMSecsSinceEpoch());
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

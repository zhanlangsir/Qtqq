#include "sendimg_job.h"

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
    sendimg_type_(sendimg_type),
    file_path_(file_path),
    data_(data)
{
}

void SendImgJob::requestDone()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
	if ( !reply->error() )
    {
		QByteArray data = reply->readAll();
		reply->close();
        reply->deleteLater();

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
		qDebug() << "error: " << reply->error() << endl;
	}

	emit sigJobDone(this, reply->error());
}

void SendImgJob::run()
{
    QString send_url;
    QString host;
    if (  sendimg_type_ == kOffpic )
    {
        send_url = "http://weboffline.ftn.qq.com/ftn_access/upload_offline_pic?time=" + QString::number(QDateTime::currentMSecsSinceEpoch());
;
        host = "weboffline.ftn.qq.com";
    }
    else if ( sendimg_type_ == kGroupImg )
    {
        send_url = "http://up.web2.qq.com/cgi-bin/cface_upload?time=" + QString::number(QDateTime::currentMSecsSinceEpoch());
;
        host = "up.web2.qq.com";
    }

    QNetworkRequest request(send_url);

    //header.setRequest("POST", send_url);
    request.setRawHeader("Host", host.toLatin1());
    request.setRawHeader("Content-Length", QString::number(data_.length()).toLatin1());
    request.setRawHeader("Cache-Control", "max-age=0");
    request.setRawHeader("Origin", "http://web.qq.com");
    request.setRawHeader("Content-Type", "multipart/form-data; boundary="+Protocol::QQProtocol::instance()->imgSender()->boundary().toLatin1());
    request.setRawHeader("Referer", "http://web.qq.com/");
    request.setRawHeader("Cookie", CaptchaInfo::instance()->cookie().toLatin1());

    QNetworkReply *reply = Protocol::QQProtocol::instance()->networkMgr()->post(request, data_);
    connect(reply, SIGNAL(finished()), this, SLOT(requestDone()));
}

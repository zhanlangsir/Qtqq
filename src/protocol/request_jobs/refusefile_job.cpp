#include "refusefile_job.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDebug>
#include <QDateTime>

#include "core/captchainfo.h"
#include "protocol/qq_protocol.h"

RefuseFileJob::RefuseFileJob(const QString &to_id, int session_id, JobType type) : 
	__JobBase(NULL, type),
    to_id_(to_id),
    session_id_(session_id)
{
}

void RefuseFileJob::requestDone(bool error)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
	if ( !reply->error() )
	{
		QByteArray data = reply->readAll();
        reply->deleteLater();

        qDebug() << "Refuse file done, data:\n" << data << endl;

        /*
        Protocol::Event *event = Protocol::EventCenter::instance()->createImgSendDoneEvent(for_, success, file_path_, img_id);
        Protocol::EventCenter::instance()->triggerEvent(event);
        */
	}
	else
	{
		qDebug() << "Refuse file for: " << to_id_ << "failed" << endl;
		qDebug() << "error: " << reply->error() << endl;
	}

	emit sigJobDone(this, error);
}

void RefuseFileJob::run()
{
    QString send_url = "/channel/refuse_file2?to=%1&lcid=%2&clientid=5412354841&psessionid=%3&t=%4";
    QString host = "d.web2.qq.com";

    QNetworkRequest request(send_url.arg(to_id_).arg(session_id_).arg(CaptchaInfo::instance()->psessionid()).arg(QDateTime::currentMSecsSinceEpoch()));
    request.setRawHeader("Host", host.toLatin1());
    request.setRawHeader("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002&callback=1&id=2");
    request.setRawHeader("Cookie", CaptchaInfo::instance()->cookie().toLatin1());

    QNetworkReply *reply = Protocol::QQProtocol::instance()->networkMgr()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(requestDone()));
}

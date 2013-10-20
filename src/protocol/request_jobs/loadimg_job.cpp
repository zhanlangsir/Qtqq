#include "loadimg_job.h"

#include <QDebug>

#include "core/talkable.h"
#include "core/captchainfo.h"
#include "protocol/event.h"
#include "protocol/event_center.h"

LoadImgJob::LoadImgJob(QString file, ImgType img_type, JobType type) : 
	__JobBase(NULL, type),
    file_(file),
    img_type_(img_type)
{
}

void LoadImgJob::reciveFile(QString file_url)
{
    QString host = getHost(file_url);

    qDebug() << file_url << endl;
    QNetworkRequest request(file_url);
    request.setRawHeader("Host", host.toLatin1());
    request.setRawHeader("Referer", "http://web.qq.com");
    request.setRawHeader("Cookie", CaptchaInfo::instance()->cookie().toLatin1());

    QNetworkReply *reply = Protocol::QQProtocol::instance()->networkMgr()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onRequestImgDone()));
}

void LoadImgJob::onRequestImgDone()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
	if ( !reply->error() )
	{
		QByteArray data = reply->readAll();
        reply->deleteLater();

        triggerEvent(data);
	}
	else
	{
		qDebug() << "request img failed! " << endl;
		qDebug() << "error: " << reply->error() << endl;
	}

	emit sigJobDone(this, reply->error());
}

void LoadImgJob::getImgUrl()
{
    QNetworkRequest request(request_url_);
    request.setRawHeader("Host", host_.toLatin1());
    request.setRawHeader("Referer", "http://web.qq.com");
    request.setRawHeader("Cookie", CaptchaInfo::instance()->cookie().toLatin1());
    qDebug() << "Get Img Url " << request.rawHeaderList() << endl;
    
    QNetworkReply *reply = Protocol::QQProtocol::instance()->networkMgr()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onGetImgUrlDone()));
}

void LoadImgJob::onGetImgUrlDone()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());

    qDebug() << "Get Image url done:\n" << endl;
    QString location = reply->rawHeader("Location");
    if ( location.isEmpty() )
    {
		QByteArray data = reply->readAll();
        reply->deleteLater();

        triggerEvent(data);
        emit sigJobDone(this, reply->error());
    }
    else
    {
        reciveFile(location);
    }
}

void LoadImgJob::run()
{
    getImgUrl();
}

QString LoadImgJob::getRequestUrl(const QString &url)
{
    int request_idx = url.indexOf("com/")+3;

    if ( request_idx == 2 )
    {
        request_idx = url.indexOf("cn/")+2;
    }

    return url.mid(request_idx);
}

QString LoadImgJob::getHost(const QString &url)
{
    QUrl _url(url);
    QString host = _url.host();

    return host;
}

void LoadOffpicJob::triggerEvent(const QByteArray &data)
{
    Protocol::Event *event = Protocol::EventCenter::instance()->createImgLoadDoneEvent(file_, to_id_, img_type_, data);
    Protocol::EventCenter::instance()->triggerEvent(event);
}

void LoadCfaceJob::triggerEvent(const QByteArray &data)
{
    Protocol::Event *event = Protocol::EventCenter::instance()->createImgLoadDoneEvent(file_, to_id_, img_type_, data);
    Protocol::EventCenter::instance()->triggerEvent(event);
}

void LoadGroupImgJob::triggerEvent(const QByteArray &data)
{
    Protocol::Event *event = Protocol::EventCenter::instance()->createImgLoadDoneEvent(file_, gid_, img_type_, data);
    Protocol::EventCenter::instance()->triggerEvent(event);
}

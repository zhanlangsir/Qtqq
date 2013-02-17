#include "loadimg_job.h"

#include <QHttpRequestHeader>
#include <QDebug>

#include "core/talkable.h"
#include "core/captchainfo.h"
#include "protocol/event.h"
#include "protocol/event_center.h"

LoadImgJob::LoadImgJob(QString file, ImgType img_type, JobType type) : 
	__JobBase(NULL, type),
	http_(this),
    file_(file),
    img_type_(img_type)
{
}

void LoadImgJob::reciveFile(QString file_url)
{
    QString host = getHost(file_url);

    QHttpRequestHeader header;
    header.setRequest("GET", getRequestUrl(file_url));
    header.addValue("Host", host);
    header.addValue("Referer", "http://web.qq.com");
    header.addValue("Cookie", CaptchaInfo::instance()->cookie());

    http_.setHost(host);
    connect(&http_, SIGNAL(done(bool)), this, SLOT(onRequestImgDone(bool)));
    http_.request(header);
}

void LoadImgJob::onRequestImgDone(bool err)
{
	if ( !err )
	{
		QByteArray data = http_.readAll();
		http_.close();

        triggerEvent(data);
	}
	else
	{
		qDebug() << "request img failed! " << endl;
		qDebug() << "error: " << http_.errorString() << endl;
	}

	http_.disconnect(this);
	emit sigJobDone(this, err);
}

void LoadImgJob::getImgUrl()
{
    QHttpRequestHeader header;
    header.setRequest("GET", request_url_);
    header.addValue("Host", host_);
    header.addValue("Referer", "http://web.qq.com");
    header.addValue("Cookie", CaptchaInfo::instance()->cookie());
    qDebug() << "Get Img Url " << header.toString() << endl;
    
    http_.setHost(host_);
    connect(&http_, SIGNAL(done(bool)), this, SLOT(onGetImgUrlDone(bool)));
    http_.request(header);
}

void LoadImgJob::onGetImgUrlDone(bool err)
{
    disconnect(&http_, SIGNAL(done(bool)), this, SLOT(onGetImgUrlDone(bool)));

    QHttpResponseHeader response = http_.lastResponse();

    qDebug() << "Get Image url done:\n" 
        << response.toString() << endl;
    QString location = response.value("Location");

    reciveFile(location);
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

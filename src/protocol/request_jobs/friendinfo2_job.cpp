#include "friendinfo2_job.h"

#include <QDateTime>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>

#include "core/captchainfo.h"
#include "protocol/qq_protocol.h"

FriendInfo2Job::FriendInfo2Job(Talkable *job_for, JobType type) :
	__JobBase(job_for->id(), type)
{
}

void FriendInfo2Job::run()
{
	QString get_friend_info_url = "http://s.web2.qq.com/api/get_friend_info2?tuin="+ id_ +"&verifysession=&code=&vfwebqq=" +
		CaptchaInfo::instance()->vfwebqq() + "&t=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());

    QNetworkRequest request(get_friend_info_url);
	request.setRawHeader("Host", "s.web2.qq.com");
	request.setRawHeader("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001&callback=1&id=1");
	request.setRawHeader("Connection", "keep-live");
	request.setRawHeader("Content-Type","utf-8");
	request.setRawHeader("Cookie", CaptchaInfo::instance()->cookie().toLatin1());

    QNetworkReply *reply = Protocol::QQProtocol::instance()->networkMgr()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(requestDone()));
}

void FriendInfo2Job::requestDone(bool error)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
	if ( !reply->error() )
	{
		QByteArray data = reply->readAll();
        reply->deleteLater();

		qDebug() << data << endl;

	}
	else
	{
		qDebug() << "request icon for " << id_ << " failed! " << endl;
		qDebug() << "error: " << reply->error() << endl;
	}

	emit sigJobDone(this, reply->error());
}

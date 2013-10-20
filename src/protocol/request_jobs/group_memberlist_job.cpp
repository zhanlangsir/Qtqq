#include "group_memberlist_job.h"

#include <QDateTime>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>

#include "core/captchainfo.h"
#include "protocol/qq_protocol.h"
#include "protocol/event_center.h"

GroupMemberListJob::GroupMemberListJob(Group *job_for, JobType type) :
    __JobBase(job_for->id(), type),
    t_type_(job_for->type()),
    gcode_(job_for->gcode())
{
}

void GroupMemberListJob::run()
{
    QString get_group_member_url = "http://s.web2.qq.com/api/get_group_info_ext2?gcode=" + gcode_ + "&vfwebqq=" +
        CaptchaInfo::instance()->vfwebqq() + "&t="+ QString::number(QDateTime::currentMSecsSinceEpoch());

    QNetworkRequest request(get_group_member_url);
    request.setRawHeader("Host", "s.web2.qq.com");
    request.setRawHeader("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001");
    request.setRawHeader("Cookie", CaptchaInfo::instance()->cookie().toLatin1());


    QNetworkReply *reply = Protocol::QQProtocol::instance()->networkMgr()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(requestDone()));
}

void GroupMemberListJob::requestDone()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
	if ( !reply->error() )
	{
		QByteArray data = reply->readAll();
        reply->deleteLater();

        Protocol::Event *event = Protocol::EventCenter::instance()->createGroupMemberListUpdateEvent(id_, t_type_, data);
        Protocol::EventCenter::instance()->triggerEvent(event);
	}
	else
	{
		qDebug() << "request group member list for " << id_ << " failed! " << endl;
		qDebug() << "error: " << reply->error() << endl;
	}

	emit sigJobDone(this, reply->error());
}

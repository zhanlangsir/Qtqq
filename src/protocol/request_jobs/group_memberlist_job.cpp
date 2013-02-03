#include "group_memberlist_job.h"

#include <QDateTime>
#include <QDebug>

#include "core/captchainfo.h"
#include "protocol/qq_protocol.h"
#include "protocol/event_center.h"

GroupMemberListJob::GroupMemberListJob(Group *job_for, JobType type) :
    __JobBase(job_for, type)
{
	connect(&http_, SIGNAL(done(bool)), this, SLOT(requestDone(bool)));
}

void GroupMemberListJob::run()
{
    QString get_group_member_url = "/api/get_group_info_ext2?gcode=" + ((Group *)for_)->gcode() + "&vfwebqq=" +
        CaptchaInfo::instance()->vfwebqq() + "&t="+ QString::number(QDateTime::currentMSecsSinceEpoch());

    QHttpRequestHeader header("GET", get_group_member_url);
    header.addValue("Host", "s.web2.qq.com");
    header.addValue("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001");
    header.addValue("Cookie", CaptchaInfo::instance()->cookie());

    http_.setHost("s.web2.qq.com");
    http_.request(header);
}

void GroupMemberListJob::requestDone(bool err)
{
	if ( !err )
	{
		QByteArray data = http_.readAll();
		http_.close();
        Protocol::Event *event = Protocol::EventCenter::instance()->createGroupMemberListUpdateEvent((Group *)for_, data);
        Protocol::EventCenter::instance()->triggerEvent(event);
	}
	else
	{
		qDebug() << "request group member list for " << for_->id() << " failed! " << endl;
		qDebug() << "error: " << http_.errorString() << endl;
	}

	http_.disconnect(this);
	emit sigJobDone(this, err);
}

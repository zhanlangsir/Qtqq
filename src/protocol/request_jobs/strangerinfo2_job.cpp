#include "strangerinfo2_job.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDateTime>
#include <QDebug>

#include "core/captchainfo.h"
#include "strangermanager/stranger_manager.h"
#include "protocol/qq_protocol.h"
#include "protocol/event.h"
#include "protocol/event_center.h"

StrangerInfo2Job::StrangerInfo2Job(Talkable *job_for, QString gid, QString code, JobType type) :
    __JobBase(job_for->id(), type),
    gid_(gid),
    code_(code),
    t_type_(job_for->type())
{
}

void StrangerInfo2Job::run()
{
    QString get_stranger_info_url = "/api/get_stranger_info2?tuin=" + id_ + "&verifysession=&gid=0&code=" + code_ + "&vfwebqq=" + CaptchaInfo::instance()->vfwebqq() + "&t=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());


    QNetworkRequest request(get_stranger_info_url);
    //http_.setHost("s.web2.qq.com");
    //header.setRequest("GET", get_stranger_info_url);
    request.setRawHeader("Host", "s.web2.qq.com");
    request.setRawHeader("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001&callback=1&id=1");
    request.setRawHeader("Connection", "keep-live");
    request.setRawHeader("Content-Type","utf-8");
    request.setRawHeader("Cookie", CaptchaInfo::instance()->cookie().toLatin1());

    QNetworkReply *reply = Protocol::QQProtocol::instance()->networkMgr()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(requestDone()));
}

void StrangerInfo2Job::requestDone()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if ( !reply->error() )
    {
        QByteArray data = reply->readAll();
        reply->close();
        reply->deleteLater();


        Protocol::Event *e = Protocol::EventCenter::instance()->createStrangerInfoDoneEvent(id_, t_type_, data);
        Protocol::EventCenter::instance()->triggerEvent(e);
    }
    else
    {
        qDebug() << "request stranger for " << id_ << " failed! " << endl;
        qDebug() << "error: " << reply->error() << endl;
    }

    emit sigJobDone(this, reply->error());
}

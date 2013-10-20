#include "sendmsg_job.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDebug>

#include "core/talkable.h"
#include "core/captchainfo.h"
#include "protocol/request_jobs/sendimg_job.h"
#include "protocol/event.h"
#include "protocol/event_center.h"
#include "protocol/msgsender.h"
#include "protocol/imgsender.h"
#include "protocol/qq_protocol.h"

SendMsgJob::SendMsgJob(Talkable *_for, const QVector<QQChatItem> &msgs, JobType type) : 
	__JobBase(_for->id(), type),
    t_type_(_for->type()),
    msgs_(msgs)
{
}

void SendMsgJob::run()
{
    sendMsg();
}

void SendMsgJob::sendMsg()
{
    QString send_url = "http://d.web2.qq.com" + getPath();
    QByteArray data = getData();

    QNetworkRequest request(send_url);
    request.setRawHeader("Host", "d.web2.qq.com");
    request.setRawHeader("Cookie", CaptchaInfo::instance()->cookie().toLatin1());
    request.setRawHeader("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002");
    request.setRawHeader("Content-Length", QString::number(data.length()).toLatin1());
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    QNetworkReply *reply = Protocol::QQProtocol::instance()->networkMgr()->post(request, data);
    connect(reply, SIGNAL(finished()), this, SLOT(requestDone()));

    qDebug() << data << endl;
    qDebug() << request.rawHeaderList() << endl;
}

void SendMsgJob::requestDone()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    QByteArray data = reply->readAll();
    qDebug() << "SendMsgJob::requestDone, " << data << endl;
    reply->deleteLater();

    Protocol::MsgSendDoneEvent *event = new Protocol::MsgSendDoneEvent(id_, t_type_, reply->error());
    if ( reply->error() )
    {
        qDebug() << "send msg for " << id_ << " failed! " << endl;
        qDebug() << "error: " << reply->error() << endl;
        event->setErrStr(QString());
        event->setMsgs(msgs_);
    }
    Protocol::EventCenter::instance()->triggerEvent(event);

    emit sigJobDone(this, reply->error());
}

void SendFriendMsgJob::run()
{
    bool has_img = false;
    foreach ( const QQChatItem &msg, msgs_ )
    {
        if ( msg.type() == QQChatItem::kFriendOffpic )
        {
            if ( !jobs_.contains(msg.content()) )
            {
                QFile img_file(msg.content());
                img_file.open(QIODevice::ReadOnly);
                QByteArray img_data = img_file.readAll();
                img_file.close();

                QByteArray data = Protocol::QQProtocol::instance()->imgSender()->createOffpicBody(msg.content(), img_data);

                jobs_.append(msg.content());
                SendImgJob *job = new SendImgJob(msg.content(), data, SendImgJob::kOffpic);
                connect(job, SIGNAL(sigJobDone(__JobBase *, bool)), this, SLOT(onImgSendDone(__JobBase *, bool)));
                job->run();

                has_img = true;
            }
        }
    }

    if ( !has_img )
        sendMsg();
}

QString SendFriendMsgJob::getPath() const
{
    return "/channel/send_buddy_msg2";
}

QByteArray SendFriendMsgJob::getData() const
{
    return Protocol::QQProtocol::instance()->msgSender()->msgToJson(id_, msgs_).toUtf8();
}

void SendFriendMsgJob::onImgSendDone(__JobBase *job, bool error)
{
    SendImgJob *imgjob = (SendImgJob *)job;
    if ( !error )
    {
        ++finish_imgjobs_;
        if ( finish_imgjobs_ == jobs_.count() )
        {
            sendMsg();
        }
    }

    job->deleteLater();
}

void SendGroupMsgJob::run()
{
    bool has_img = false;
    foreach ( const QQChatItem &msg, msgs_ )
    {
        if ( msg.type() == QQChatItem::kGroupChatImg )
        {
            if ( !jobs_.contains(msg.content()) )
            {
                QFile img_file(msg.content());
                img_file.open(QIODevice::ReadOnly);
                QByteArray img_data = img_file.readAll();
                img_file.close();

                QByteArray data = Protocol::QQProtocol::instance()->imgSender()->createGroupImgBody(msg.content(), img_data);

                jobs_.append(msg.content());
                SendImgJob *job = new SendImgJob(msg.content(), data, SendImgJob::kGroupImg);
                connect(job, SIGNAL(sigJobDone(__JobBase *, bool)), this, SLOT(onImgSendDone(__JobBase *, bool)));
                job->run();

                has_img = true;
            }
        }
    }

    if ( !has_img )
        sendMsg();
}

void SendGroupMsgJob::onImgSendDone(__JobBase *job, bool error)
{
    SendImgJob *imgjob = (SendImgJob *)job;
    if ( !error )
    {
        ++finish_imgjobs_;
        if ( finish_imgjobs_ == jobs_.count() )
        {
            sendMsg();
        }
    }
    job->deleteLater();
}

QString SendGroupMsgJob::getPath() const
{
    return "/channel/send_qun_msg2";
}

QByteArray SendGroupMsgJob::getData() const
{
    return Protocol::QQProtocol::instance()->msgSender()->groupMsgToJson(id_, gcode_, msgs_).toUtf8();
}



QString SendSessMsgJob::getPath() const
{
    return "/channel/send_sess_msg2";
}

QByteArray SendSessMsgJob::getData() const
{
    return Protocol::QQProtocol::instance()->msgSender()->sessMsgToJson(id_, group_->id(), msgs_).toLatin1();
}

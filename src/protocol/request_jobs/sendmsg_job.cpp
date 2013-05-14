#include "sendmsg_job.h"

#include <QHttpRequestHeader>
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
	http_(this),
    msgs_(msgs)
{
	connect(&http_, SIGNAL(done(bool)), this, SLOT(requestDone(bool)));
}

void SendMsgJob::requestDone(bool error)
{
	if ( !error )
	{
		QByteArray data = http_.readAll();
        qDebug() << "SendMsgJob::requestDone, " << data << endl;
		http_.close();

        Protocol::Event *event = Protocol::EventCenter::instance()->createMsgSendDoneEvent(id_, t_type_, data);
        Protocol::EventCenter::instance()->triggerEvent(event);
	}
	else
	{
		qDebug() << "send msg for " << id_ << " failed! " << endl;
		qDebug() << "error: " << http_.errorString() << endl;
	}

	http_.disconnect(this);
	emit sigJobDone(this, error);
}

void SendMsgJob::run()
{
    sendMsg();
}

void SendMsgJob::sendMsg()
{
    QString send_url = getPath();
    QByteArray data = getData();

    QHttpRequestHeader header;
    header.setRequest("POST", send_url);
    header.setValue("Host", "d.web2.qq.com");
    header.setValue("Cookie", CaptchaInfo::instance()->cookie());
    header.setValue("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002");
    header.setValue("Content-Length", QString::number(data.length()));
    header.setValue("Content-Type", "application/x-www-form-urlencoded");

    http_.setHost("d.web2.qq.com");
    http_.request(header, data);

    qDebug() << data << endl;
    qDebug() << header.toString() << endl;
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
    return Protocol::QQProtocol::instance()->msgSender()->msgToJson(id_, msgs_).toAscii();
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
    return Protocol::QQProtocol::instance()->msgSender()->groupMsgToJson(id_, gcode_, msgs_).toAscii();
}



QString SendSessMsgJob::getPath() const
{
    return "/channel/send_sess_msg2";
}

QByteArray SendSessMsgJob::getData() const
{
    return Protocol::QQProtocol::instance()->msgSender()->sessMsgToJson(id_, group_->id(), msgs_).toAscii();
}

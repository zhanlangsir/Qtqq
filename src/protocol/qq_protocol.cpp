#include "qq_protocol.h"

#include <QDebug>

#include "roster/roster.h"
#include "core/talkable.h"
#include "storage/storage.h"
#include "protocol/request_jobs/icon_job.h"
#include "protocol/request_jobs/strangerinfo2_job.h"
#include "protocol/request_jobs/sendimg_job.h"
#include "protocol/request_jobs/sendmsg_job.h"
#include "protocol/request_jobs/friendinfo2_job.h"
#include "protocol/request_jobs/group_memberlist_job.h"
#include "protocol/pollthread.h"

Protocol::QQProtocol* Protocol::QQProtocol::instance_ = NULL;

Protocol::QQProtocol::QQProtocol() :
	poll_thread_(new Protocol::PollThread(this)),
    imgsender_(new ImgSender())
{
	connect(poll_thread_, SIGNAL(newMsgArrive(QByteArray)), this, SIGNAL(newQQMsg(QByteArray)));
}

Protocol::QQProtocol::~QQProtocol()
{
	stop();
}

void Protocol::QQProtocol::run()
{
	poll_thread_->start();
}


void Protocol::QQProtocol::stop()
{
	poll_thread_->stop();
}


void Protocol::QQProtocol::requestIconFor(Talkable *req_for)
{
	__JobBase *job = new IconJob(req_for);
    runJob(job);
}


void Protocol::QQProtocol::runJob(__JobBase *job)
{
	connect(job, SIGNAL(sigJobDone(__JobBase*, bool)), this, SLOT(slotJobDone(__JobBase*, bool)));
	job->run();
}

void Protocol::QQProtocol::requestSingleLongNick(QString id)
{

}

void Protocol::QQProtocol::requestContactInfo(QString id)
{

}

void Protocol::QQProtocol::requestFriendInfo2(Talkable *job_for)
{
	FriendInfo2Job *job = new FriendInfo2Job(job_for);
    runJob(job);
}

void Protocol::QQProtocol::requestStrangerInfo2(Talkable *job_for)
{
	requesting_.insert(JT_StrangerInfo2, job_for->id());
	StrangerInfo2Job *job = new StrangerInfo2Job(job_for);
    runJob(job);
}

void Protocol::QQProtocol::requestGroupMemberList(Group *job_for)
{
    GroupMemberListJob *job = new GroupMemberListJob(job_for);
    runJob(job);
}

void Protocol::QQProtocol::slotJobDone(__JobBase* job, bool error)
{
    requesting_.values(job->type()).removeOne(job->requesterId());

    job->deleteLater();
}

void Protocol::QQProtocol::sendImg(Talkable *sender, QString file_path, QByteArray data)
{
   QByteArray sended_data = imgsender_->prepareSendingData(sender, file_path, data);
   qDebug() << "img data\n" << sended_data << endl;
   SendImgJob *job = new SendImgJob(sender, file_path, sended_data);

   runJob(job);
}

void Protocol::QQProtocol::sendGroupImg()
{

}

void Protocol::QQProtocol::sendMsg(Talkable *to, const QVector<QQChatItem> &msgs)
{
    QString sended_data;
    if ( to->type() == Talkable::kContact )
        sended_data = MsgSender::msgToJson((Contact *)to, msgs);
    else
        sended_data = MsgSender::groupMsgToJson((Group *)to, msgs);

    SendMsgJob *job = new SendMsgJob(to, sended_data);

    runJob(job);
}

void Protocol::QQProtocol::sendGroupMsg(const QVector<QQChatItem> &msgs)
{

}

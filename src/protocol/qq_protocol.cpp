#include "qq_protocol.h"

#include <QDebug>

#include "roster/roster.h"
#include "core/talkable.h"
#include "storage/storage.h"
#include "protocol/imgsender.h"
#include "protocol/filesender.h"
#include "protocol/request_jobs/icon_job.h"
#include "protocol/request_jobs/strangerinfo2_job.h"
#include "protocol/request_jobs/sendimg_job.h"
#include "protocol/request_jobs/sendfile_job.h"
#include "protocol/request_jobs/sendmsg_job.h"
#include "protocol/request_jobs/loadimg_job.h"
#include "protocol/request_jobs/filerecive_job.h"
#include "protocol/request_jobs/friendinfo2_job.h"
#include "protocol/request_jobs/group_memberlist_job.h"
#include "protocol/request_jobs/refusefile_job.h"
#include "protocol/pollthread.h"

Protocol::QQProtocol* Protocol::QQProtocol::instance_ = NULL;

Protocol::QQProtocol::QQProtocol() :
	poll_thread_(new Protocol::PollThread(this)),
    imgsender_(new ImgSender()),
    filesender_(new FileSender()),
    msgsender_(new MsgSender())
{
	connect(poll_thread_, SIGNAL(newMsgArrive(QByteArray)), this, SIGNAL(newQQMsg(QByteArray)));
}

Protocol::QQProtocol::~QQProtocol()
{
	stop();
    reciving_jobs_.clear();
    sending_jobs_.clear();
    requesting_.clear();
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
    requesting_[job->type()].push_back(job->requesterId());
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

void Protocol::QQProtocol::requestStrangerInfo2(Talkable *job_for, QString gid, bool group_request)
{
    QString code = "";
    if ( group_request )
        code = "group_request_join-" + gid;

	StrangerInfo2Job *job = new StrangerInfo2Job(job_for, gid, code);
    runJob(job);
}

void Protocol::QQProtocol::requestGroupMemberList(Group *job_for)
{
    GroupMemberListJob *job = new GroupMemberListJob(job_for);
    runJob(job);
}

/*
void Protocol::QQProtocol::sendImg(Talkable *sender, QString file_path, QByteArray data)
{
   QByteArray sended_data = imgsender_->prepareSendingData(sender, file_path, data);
   SendImgJob *job = new SendImgJob(sender->id(), file_path, sended_data);

   runJob(job);
}
*/

void Protocol::QQProtocol::sendMsg(Talkable *to, Group *group, const QVector<QQChatItem> &msgs)
{
    __JobBase *job = NULL;
    if ( to->type() == Talkable::kContact || to->type() == Talkable::kStranger )
    {
        job = new SendFriendMsgJob(to, msgs);
    }
    else if ( to->type() == Talkable::kSessStranger )
    {
        job = new SendSessMsgJob(to, group, msgs);
    }
    else if ( to->type() == Talkable::kGroup )
    {
        job = new SendGroupMsgJob(to, msgs);
    }

    runJob(job);
}

void Protocol::QQProtocol::sendFile(const QString &file_path, const QString &to_id, const QByteArray &data)
{
    QByteArray body = filesender_->createFileData(file_path, data);
    SendFileJob *job = new SendFileJob(to_id, file_path, body, SendFileJob::kFile);
    connect(job, SIGNAL(sendFileProgress(QString, int, int)), this, SIGNAL(sendFileProgress(QString, int, int)));

    sending_jobs_.insert(file_path, job);

    runJob(job);
}

void Protocol::QQProtocol::sendOffFile(const QString &file_path, const QString &to_id, const QByteArray &data)
{
    QByteArray body = filesender_->createOffFileData(file_path, to_id, data);
    SendFileJob *job = new SendFileJob(to_id, file_path, body, SendFileJob::kOffFile);
    connect(job, SIGNAL(sendFileProgress(QString, int, int)), this, SIGNAL(sendFileProgress(QString, int, int)));

    sending_jobs_.insert(file_path, job);

    runJob(job);
}

void Protocol::QQProtocol::reciveFile(int session_id, QString file_name, QString to)
{
    FileReciveJob *job = new FileReciveJob(session_id, file_name, to);
    connect(job, SIGNAL(fileTransferProgress(int, int, int)), this, SIGNAL(fileTransferProgress(int, int, int)));

    reciving_jobs_.insert(session_id, job);

    runJob(job);
}

void Protocol::QQProtocol::pauseRecvFile(int session_id)
{
    if ( reciving_jobs_.contains(session_id) )
    {
        reciving_jobs_[session_id]->stop();
    }
}

void Protocol::QQProtocol::pauseSendFile(QString file_path)
{
    if ( sending_jobs_.contains(file_path) )
    {
        sending_jobs_[file_path]->stop();
    }
}

void Protocol::QQProtocol::loadFriendOffpic(QString file, QString to_id)
{
    __JobBase *job = new LoadOffpicJob(file, to_id);
    runJob(job);
}

void Protocol::QQProtocol::loadFriendCface(const QString &file, const QString &to_id, const QString &msg_id)
{
    __JobBase *job = new  LoadCfaceJob(file, to_id, msg_id);
    runJob(job);
}

void Protocol::QQProtocol::loadGroupImg(QString gid, const QString &file, QString id, const QString &gcode, QString fid, QString ip, QString port, const QString &time)
{
	__JobBase *job = new LoadGroupImgJob(gid, file, id, gcode, fid, ip, port, time);
    runJob(job);
}

void Protocol::QQProtocol::refuseRecvFile(QString to_id, int session_id)
{
    __JobBase *job = new RefuseFileJob(to_id, session_id); 

    runJob(job);
}

void Protocol::QQProtocol::slotJobDone(__JobBase* job, bool error)
{
    switch ( job->type() )
    {
        case JT_FileRecive:
            reciving_jobs_.remove(((FileReciveJob *)job)->sessionId());
            break;
        case JT_SendFile:
            sending_jobs_.remove(((SendFileJob *)job)->filePath());
            break;
        case JT_Icon:
            if ( job->jobFor() )
                requesting_[job->type()].removeOne(job->requesterId());
            else
                qDebug() << "Wraning: lost one job, job type: " << job->type() << endl;
            break;
        default:
            break;
    }

    job->deleteLater();
}

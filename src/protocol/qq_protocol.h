#ifndef QQ_PROTOCOL_H
#define QQ_PROTOCOL_H

#include <QObject>
#include <QList>
#include <QMap>

#include "protocol/request_jobs/job_base.h"
#include "protocol/imgsender.h"
#include "protocol/msgsender.h"

class __JobBase;
typedef JobType RequestType;

namespace Protocol
{
class QQProtocol;
	class PollThread;
};

class Talkable;
class FileReciveJob;

class Protocol::QQProtocol : public QObject
{
	Q_OBJECT
signals:
	void newQQMsg(QByteArray msg);

    void fileTransferProgress(int session_id, int recived_byte, int totol_byte);

public:
	~QQProtocol();
	static QQProtocol* instance()
	{
		if ( !instance_ )
			instance_ = new QQProtocol();

		return instance_;
	}

	void run();
	void stop();

	void requestIconFor(Talkable *req_for);

	void requestSingleLongNick(QString id);
	void requestContactInfo(QString id);
	void requestFriendInfo2(Talkable *job_for);
	void requestStrangerInfo2(Talkable *job_for, QString gid, bool group_request);
    void requestGroupMemberList(Group *job_for);
    void requestGroupSig();

    void reciveFile(int session_id, QString file_name, QString to);
    void parseTransferFile(int session_id);

    void sendImg(Talkable *sender, QString file_name, QByteArray data);
    void sendGroupImg();
    void sendMsg(Talkable *to, const QVector<QQChatItem> &msgs); 
    void sendGroupMsg(const QVector<QQChatItem> &msgs); 

	bool isRequesting(QString id, RequestType type)
	{
		return requesting_.values(type).contains(id);
	}

    ImgSender *imgSender() const
    { return imgsender_; }

private slots:
	void slotJobDone(__JobBase* job, bool error);

private:
    void runJob(__JobBase *job);

private:
	QMap<JobType, QString> requesting_;
    QMap<int, FileReciveJob *> reciving_jobs_;
	Protocol::PollThread *poll_thread_;

    ImgSender *imgsender_;
    //MsgSender *msgsender_;

private:
	QQProtocol(); 
	QQProtocol(const QQProtocol&);
	QQProtocol& operator=(const QQProtocol&);

	static QQProtocol* instance_;
};

#endif //QQ_PROTOCOL_H

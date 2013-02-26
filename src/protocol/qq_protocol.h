#ifndef QQ_PROTOCOL_H
#define QQ_PROTOCOL_H

#include <QObject>
#include <QList>
#include <QMap>

#include "protocol/request_jobs/job_base.h"
#include "protocol/msgsender.h"

class __JobBase;
typedef JobType RequestType;

namespace Protocol
{
    class QQProtocol;
	class PollThread;
    class ImgSender;
    class FileSender;
};

class Talkable;
class FileReciveJob;
class SendFileJob;

class Protocol::QQProtocol : public QObject
{
	Q_OBJECT
signals:
	void newQQMsg(QByteArray msg);

    void fileTransferProgress(int session_id, int recived_byte, int totol_byte);
    void sendFileProgress(QString file, int send_byte, int totol_byte);

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
    void refuseRecvFile(QString to_id, int session_id);

    void pauseRecvFile(int session_id);
    void pauseSendFile(QString file_path);

    //void sendImg(Talkable *sender, QString file_name, QByteArray data);
    //void sendGroupImg();
    void sendMsg(Talkable *to, Group *gruop, const QVector<QQChatItem> &msgs); 
    void sendGroupMsg(const QVector<QQChatItem> &msgs); 

    void sendFile(const QString &file_path, const QString &to_id, const QByteArray &data);
    void sendOffFile(const QString &file_paht, const QString &to_id, const QByteArray &data);

    void loadFriendOffpic(QString file, QString to_id);
    void loadFriendCface(const QString &file, const QString &to_id, const QString &msg_id);
    void loadGroupImg(QString gid, const QString &file, QString id, const QString &gcode, QString fid, QString ip, QString port, const QString &time);

	bool isRequesting(QString id, RequestType type)
	{
		return requesting_[type].contains(id);
	}

    Protocol::ImgSender *imgSender() const
    { return imgsender_; }
    Protocol::FileSender *fileSender() const
    { return filesender_; }
    Protocol::MsgSender *msgSender() const
    { return msgsender_; }

private slots:
	void slotJobDone(__JobBase* job, bool error);

private:
    void runJob(__JobBase *job);

private:
    QMap<JobType, QList<QString> > requesting_;
	//QMap<JobType, QString> requesting_;
    QMap<int, FileReciveJob *> reciving_jobs_;
    QMap<QString, SendFileJob *> sending_jobs_;

	Protocol::PollThread *poll_thread_;

    ImgSender *imgsender_;
    FileSender *filesender_;
    MsgSender *msgsender_;

private:
	QQProtocol(); 
	QQProtocol(const QQProtocol&);
	QQProtocol& operator=(const QQProtocol&);

	static QQProtocol* instance_;
};

#endif //QQ_PROTOCOL_H

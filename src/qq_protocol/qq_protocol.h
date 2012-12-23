#ifndef QQ_PROTOCOL_H
#define QQ_PROTOCOL_H

#include <QObject>
#include <QList>
#include <QMap>

#include "qq_protocol/request_jobs/job_base.h"
#include "utils/request_callback.h"

class __JobBase;
typedef JobType RequestType;

namespace Protocol
{
	class QQProtocol;
	class PollThread;
};

class Protocol::QQProtocol : public QObject
{
	Q_OBJECT
signals:
	void newQQMsg(QByteArray msg);

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

	void requestIconFor(QString requester_id, RequestCallbackBase* callback);
	void requestSingleLongNick(QString id, RequestCallbackBase* callback);
	void requestContactInfo(QString id, RequestCallbackBase* callback);
	void requestFriendInfo2(QString id, RequestCallbackBase *callback);
	void requestStrangerInfo2(QString id, QString gid, RequestCallbackBase *callback);
	bool isRequesting(QString id, RequestType type)
	{
		return requesting_.values(type).contains(id);
	}

private slots:
	void slotJobDone(__JobBase* job, bool error);

private:
	QMap<JobType, QString> requesting_;
	Protocol::PollThread *poll_thread_;

private:
	QQProtocol(); 
	QQProtocol(const QQProtocol&);
	QQProtocol& operator=(const QQProtocol&);

	static QQProtocol* instance_;
};

#endif //QQ_PROTOCOL_H

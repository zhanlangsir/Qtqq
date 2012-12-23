#ifndef REQUEST_CALLBACK_H
#define REQUEST_CALLBACK_H

#include <QObject>
#include <QByteArray>

#include "storage/storage.h"
#include "core/talkable.h"
#include "strangermanager/stranger_manager.h"

class RequestCallbackBase : public QObject
{
public:
	RequestCallbackBase(QString id) : id_(id) {}

	virtual void callback() = 0;

protected:
	QString id_;
};

class IconRequestCallback : public RequestCallbackBase
{
	Q_OBJECT
signals:
	void sigRequestDone(QString icon_id, QByteArray icon_data);

public:
	IconRequestCallback(QString icon_id) : RequestCallbackBase(icon_id) {}

	virtual void callback()
	{
		QByteArray icon_data = Storage::instance()->takeIcon(id_);
		emit sigRequestDone(id_, icon_data);
	}
};


//SLN = single lonk nick
class SLNRequestCallback : public RequestCallbackBase
{
	Q_OBJECT
signals:
	void sigRequestDone(QString id, QString single_long_nick);

public:
	SLNRequestCallback(QString id) : RequestCallbackBase(id) {}

	virtual void callback()
	{
		QString sln = Storage::instance()->takeSingleLongNick(id_);
		emit sigRequestDone(id_, sln);
	}
};

class FriendInfo2RequestCallback : public RequestCallbackBase
{
	Q_OBJECT
signals:
	void sigRequestDone(QString id, Contact *contact_info);

public:
	FriendInfo2RequestCallback(QString id) : RequestCallbackBase(id) {}

	virtual void callback()
	{
		Contact *contact = StrangerManager::instance()->strangerInfo(id_);
		emit sigRequestDone(id_, contact);
	}
};

class StrangerInfo2RequestCallback : public RequestCallbackBase
{
	Q_OBJECT
signals:
	void sigRequestDone(QString id, Contact *contact_info);

public:
	StrangerInfo2RequestCallback(QString id) : RequestCallbackBase(id) {}

	virtual void callback()
	{
		Contact *contact = StrangerManager::instance()->strangerInfo(id_);
		emit sigRequestDone(id_, contact);
	}
};

#endif //REQUEST_CALLBACK_H

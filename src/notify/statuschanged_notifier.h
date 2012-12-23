#ifndef STATUS_CHANGED_NOTIFIER_H
#define STATUS_CHANGED_NOTIFIER_H

#include <QObject>

#include "utils/contact_status.h"

class StatusChangedNotifier : public QObject
{
	Q_OBJECT
signals:
	void statusChanged(QString id, ContactStatus status, ContactClientType type)

public:
	static StatusChangedNotifier *instance()
	{
		if ( !instance_ )
			instance_ = new StatusChangedNotifier();
		
		return instance_;
	}

public slots:
	void slotNewStatusMsg(ShareQQMsgPtr msg)
	{
	}

private:
	StatusChangedNotifier(QObject *parent = NULL) : 
		QObject(parent)
	{
	}
	StatusChangedNotifier(const StatusChangedNotifier &);
	StatusChangedNotifier &operator=(const StatusChangedNotifier &);

	static StatusChangedNotifier *instance_;
};

#endif //STATUS_CHANGED_NOTIFIER_H

#ifndef STRANGER_MANAGER_H
#define STRANGER_MANAGER_H

#include <QObject>
#include <QString>
#include <QVector>

#include "core/qqmsg.h"

class Contact;

class StrangerManager : public QObject
{
	Q_OBJECT
signals:
	void newStrangerInfo(QString id, Contact *stranger);
	void newStrangerIcon(QString id, QPixmap icon);

public:
	~StrangerManager();
	static StrangerManager *instance()
	{
		if ( !instance_ )
			instance_ = new StrangerManager();

		return instance_;
	}

	bool hasStrangerInfo(QString id) const;
	Contact *strangerInfo(QString id) const;
	void addStrangerInfo(Contact *info);
	void parseStranger(const QByteArray &array);

	void clean();

private slots:
	void onNewSessMsg(ShareQQMsgPtr msg);
	void onNewSystemMsg(ShareQQMsgPtr msg);

	void onInfoRequestDone(QString id, Contact *stranger);
	void onIconRequestDone(QString id, QByteArray icon_data);

private:
	Contact *find(QString id) const;

private:
	QVector<Contact *> strangers_;

private:
	StrangerManager();
	StrangerManager(const StrangerManager &);
	StrangerManager &operator=(const StrangerManager &);

	static StrangerManager *instance_;
};

#endif //STRANGER_MANAGER_H

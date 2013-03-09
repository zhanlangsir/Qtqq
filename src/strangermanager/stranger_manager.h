#ifndef STRANGER_MANAGER_H
#define STRANGER_MANAGER_H

#include <QObject>
#include <QString>
#include <QList>

#include "interfaces/iobserver.h"
#include "core/qqmsg.h"

class Contact;

class StrangerManager : public QObject, public IObserver
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

    Contact *takeStranger(const QString &id);
    Contact *addStranger(const QString &id, const QString &gid = QString(), Talkable::TalkableType type = Talkable::kSessStranger );

	bool hasStrangerInfo(QString id) const;
    Contact *stranger(const QString &id) const;
	void clean();

protected:
    virtual void onNotify(Protocol::Event *event);

private slots:
    void onNewSessChatMsg(ShareQQMsgPtr msg);
	void onNewSystemMsg(ShareQQMsgPtr msg);

private:
	void updateStranger(const QByteArray &array, Contact *_for);

private:
	QList<Contact *> strangers_;

private:
	StrangerManager();
	StrangerManager(const StrangerManager &);
	StrangerManager &operator=(const StrangerManager &);

	static StrangerManager *instance_;
};

#endif //STRANGER_MANAGER_H

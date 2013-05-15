#ifndef RECENT_MODEL_H
#define RECENT_MODEL_H

#include <QAbstractItemModel>
#include <QByteArray>
#include <QList>

#include "rostermodel/roster_model_base.h"
#include "core/talkable.h"
#include "core/qqmsg.h"

class RosterIndex;
class Contact;
class Group;

class RecentModel : public __RosterModelBase
{
	Q_OBJECT
public:
	RecentModel(QObject *parent = NULL);
	~RecentModel();

	void parseRecentContact(QByteArray &array);
	void clean();

public slots:
	void slotMsgSended(QString id);
	void slotNewChatMsg(ShareQQMsgPtr msg);
	void slotTalkableDataChanged(QString id, QVariant data, TalkableDataRole role);
	void onDoubleClicked(const QModelIndex &index);

private:
	Talkable *talkable(QString id) const;
	RosterIndex *findIndexById(QString id) const;

	RosterIndex *createIndexFromContact(Contact *contact) const;
	RosterIndex *createIndexFromGroup(Group *group) const;
	void improveIndex(QString id);

private:
	QList<RosterIndex *> recents_;	
};

#endif //RECENT_MODEL_H

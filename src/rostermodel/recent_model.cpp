#include "recent_model.h"

#include <QModelIndex>

#include "json/json.h"

#include "roster/roster.h"
#include "rostermodel/roster_index.h"
#include "chatwidget/chatdlg_manager.h"

RecentModel::RecentModel(QObject *parent) : __RosterModelBase(parent)
{
	Roster *roster = Roster::instance();
	connect(roster, SIGNAL(sigTalkableDataChanged(QString, QVariant, TalkableDataRole)), this, SLOT(slotTalkableDataChanged(QString, QVariant, TalkableDataRole)));
}

RecentModel::~RecentModel()
{
	clean();
}

void RecentModel::parseRecentContact(QByteArray &array)
{
	Json::Reader reader;
	Json::Value root;

	if (!reader.parse(QString(array).toStdString(), root, false))
	{
		return;
	}

	const Json::Value result = root["result"];

	Roster *roster = Roster::instance();

	for (unsigned int i = 0; i < result.size(); ++i)
	{
		QString id = QString::number(result[i]["uin"].asLargestInt());
		int type = result[i]["type"].asInt();

		if (type == 0)  
		{
			Contact *contact = roster->contact(id);
			if ( !contact )
				continue;

			RosterIndex *index = createIndexFromContact(contact);
			root_->childs().push_back(index);
		}
		else if(type == 1)  
		{
			Group *group = roster->group(id);
			if ( !group )
				continue;

			RosterIndex *index = createIndexFromGroup(group);

			root_->childs().push_back(index);
		}
	}
}

RosterIndex *RecentModel::createIndexFromContact(Contact *contact) const
{
	return new TalkableIndex(contact, RIT_Contact);
}

RosterIndex *RecentModel::createIndexFromGroup(Group *group) const
{
	return new TalkableIndex(group, RIT_Group);
}

void RecentModel::slotNewChatMsg(ShareQQMsgPtr msg)
{
	improveIndex(msg->talkTo());
}

void RecentModel::slotMsgSended(QString id)
{
	improveIndex(id);
}

void RecentModel::improveIndex(QString id)
{
	RosterIndex *roster_index = findIndexById(id);
	int index = root_->childs().indexOf(roster_index);
	if ( roster_index )
	{
		beginRemoveRows(QModelIndex(), index, index);
		root_->childs().removeAt(index);
		endRemoveRows();

		beginInsertRows(QModelIndex(), 0, 0);
		root_->childs().push_front(roster_index);
		endInsertRows();

		return;
	}

	Roster *roster = Roster::instance();
	Talkable *talkable = roster->talkable(id);
	if ( !talkable )
		return;

	if ( talkable->type() == Talkable::kContact )
	{
		Contact *contact = roster->contact(id);

		RosterIndex *newIndex = new TalkableIndex(contact, RIT_Contact);
		root_->childs().push_front(newIndex);
	}
	else if ( talkable->type() == Talkable::kGroup )
	{
		Group *group = roster->group(id);

		RosterIndex *newIndex = new TalkableIndex(group, RIT_Group);
		root_->childs().push_front(newIndex);
	}
}

RosterIndex *RecentModel::findIndexById(QString id) const
{
	foreach( RosterIndex *index, root_->childs())
	{
		if ( index->id() == id )
			return index;
	}
	return NULL;
}

void RecentModel::slotTalkableDataChanged(QString id, QVariant data, TalkableDataRole role)
{
	RosterIndex *index = findIndexById(id);
	if ( index )
	{
		QModelIndex model_index = modelIndexByRosterIndex(index);
		emit dataChanged(model_index, model_index);
	}
}


void RecentModel::onDoubleClicked(const QModelIndex &index)
{
	/*
	 * now, all code in this function is the same 
	 * with RosterModel::onDoubleclicked()
	 * 
	 */
	ChatDlgManager *chat_mgr = ChatDlgManager::instance();

	QModelIndex real_index = proxy_ ? proxy_->mapToSource(index) : index;

	RosterIndex *roster_index = rosterIndexByModelIndex(real_index);

	RosterIndexType type = roster_index->type();
	if ( type == RIT_Contact )
	{
		QString id = roster_index->id();
		chat_mgr->openFriendChatDlg(id);
	}
	else if ( type == RIT_Group )
	{
		QString id = roster_index->id();
		QString gcode = roster_index->gcode();
		chat_mgr->openGroupChatDlg(id, gcode);
	}
}

void RecentModel::clean()
{
	foreach ( RosterIndex *index, root_->childs() )
	{
		delete index;
		index = NULL;
	}

	root_->childs().clear();
	recents_.clear();
}

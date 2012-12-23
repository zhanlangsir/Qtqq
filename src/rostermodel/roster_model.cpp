#include "roster_model.h"

#include "chatwidget/chatdlg_manager.h"
#include "rostermodel/roster_index.h"

RosterModel::RosterModel(QObject *parent) : __RosterModelBase(parent)
{
}


RosterModel::~RosterModel()
{
	clean();
}


void RosterModel::slotNewCategoryItem(const Category *cat)
{
	RosterIndex *index = new RosterIndex(RIT_Category);
	index->setData(TDR_Name, cat->name());
	index->setData(TDR_CategoryIndex, cat->index());

	root_->appendChild(index);
	indexs_.insert(QString::number(cat->index()), index);
}


void RosterModel::slotNewContactItem(const Contact *contact)
{
	RosterIndex *index = new RosterIndex(RIT_Contact);
	index->setData(TDR_Name, contact->name());
	index->setData(TDR_Id, contact->id());
	index->setData(TDR_Status, QVariant::fromValue<ContactStatus>(contact->status()));

	if ( !contact->markname().isEmpty() )
		index->setData(TDR_Markname, contact->markname());

	RosterIndex *cat = findCategoryIndex(contact->category()->index());
	indexs_.insert(contact->id(), index);
	assert(cat);
	cat->appendChild(index);

	QModelIndex changed_index = modelIndexByRosterIndex(cat);
	dataChanged(changed_index, changed_index);
}	


void RosterModel::slotNewGroupItem(const Group *group)
{
	RosterIndex *index = new RosterIndex(RIT_Group);
	index->setData(TDR_Id, group->id());
	index->setData(TDR_Gcode, group->gcode());
	index->setData(TDR_Name, group->name());

	indexs_.insert(group->id(), index);
	root_->appendChild(index);
}


RosterIndex *RosterModel::findCategoryIndex(int cat_index)
{
	if ( root_->childCount() == 0 || root_->child(0)->type() != RIT_Category )
		return NULL;


	for ( int i = 0; i < root_->childCount(); ++i )
	{
		RosterIndex *cat = root_->child(i);
	}

	for ( int i = 0; i < root_->childCount(); ++i )
	{
		RosterIndex *cat = root_->child(i);
		if ( cat->data(TDR_CategoryIndex).toInt() == cat_index )
		{
			return cat;
		}
	}

	return NULL;
}


void RosterModel::slotTalkableDataChanged(QString id, QVariant data, TalkableDataRole role)
{
	RosterIndex *index = findRosterIndexById(id);
	if ( !index )
		return;

	index->setData(role, data);
	QModelIndex changed_index = modelIndexByRosterIndex(index);
	dataChanged(changed_index, changed_index);
}


void RosterModel::slotCategoryDataChanged(int index, QVariant data, TalkableDataRole role)
{
	RosterIndex *roster_index = findCategoryIndex(index);
	assert(roster_index);
	roster_index->setData(role, data);

	QModelIndex changed_index = modelIndexByRosterIndex(roster_index);
	dataChanged(changed_index, changed_index);
}

RosterIndex *RosterModel::findRosterIndexById(QString id)
{
	foreach (RosterIndex *index, indexs_)
	{
		if ( index->data(TDR_Id).toString() == id )
			return index;
	}

	return NULL;
}

void RosterModel::slotOnDoubleclicked(const QModelIndex &index)
{
	ChatDlgManager *chat_mgr = ChatDlgManager::instance();

	QModelIndex real_index = proxy_ ? proxy_->mapToSource(index) : index;

	RosterIndex *roster_index = rosterIndexByModelIndex(real_index);

	RosterIndexType type = roster_index->type();
	if ( type == RIT_Contact )
	{
		QString id = roster_index->data(TDR_Id).toString();
		chat_mgr->openFriendChatDlg(id);
	}
	else if ( type == RIT_Group )
	{
		QString id = roster_index->data(TDR_Id).toString();
		QString gcode = roster_index->data(TDR_Gcode).toString();
		chat_mgr->openGroupChatDlg(id, gcode);
	}
}


void RosterModel::clean()
{
	foreach ( RosterIndex *index, indexs_ )
	{
		delete index;
		index = NULL;
	}

	root_->childs().clear();
	indexs_.clear();
}

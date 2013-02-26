#include "roster_model.h"

#include "chatwidget/chatdlg_manager.h"
#include "roster/roster.h"
#include "rostermodel/roster_index.h"

RosterModel::RosterModel(QObject *parent) : __RosterModelBase(parent)
{
}

RosterModel::~RosterModel()
{
	clean();
}

void RosterModel::addCategoryItem(const Category *cat)
{
	RosterIndex *index = new RosterIndex(RIT_Category);
	index->setData(TDR_Name, cat->name());
	index->setData(TDR_CategoryIndex, cat->index());

	root_->appendChild(index);
	indexs_.insert(QString::number(cat->index()), index);
}

void RosterModel::addContactItem(const Contact *contact)
{
    RosterIndexType type;
    switch ( contact->type() )
    {
        case Talkable::kContact:
            type = RIT_Contact;
            break;
        case Talkable::kStranger:
            type = RIT_Stranger;
            break;
        case Talkable::kSessStranger:
            type = RIT_Sess;
            break;
        default:
            qDebug() << "Wrong type on RosterModel: " << contact->type() << endl;
            return;
    }

	RosterIndex *index = new RosterIndex(type);
	index->setData(TDR_Name, contact->name());
	index->setData(TDR_Id, contact->id());
	index->setData(TDR_Status, QVariant::fromValue<ContactStatus>(contact->status()));

	if ( !contact->markname().isEmpty() )
		index->setData(TDR_Markname, contact->markname());

    if ( !contact->avatar().isNull() )
        index->setData(TDR_Avatar, contact->avatar());

    Category *cat = contact->category();
    if ( cat )
    {
        RosterIndex *cat_idx = findCategoryIndex(contact->category()->index());
        assert(cat_idx);

        beginInsertRows(modelIndexByRosterIndex(cat_idx), cat_idx->childCount(), cat_idx->childCount());
        cat_idx->appendChild(index);
        endInsertRows();
    }
    else
    {
        QModelIndex changed_index = modelIndexByRosterIndex(index);
        beginInsertRows(QModelIndex(), root_->childCount(), root_->childCount());
        root_->appendChild(index);
        endInsertRows();
    }
    indexs_.insert(contact->id(), index);
}	

void RosterModel::addGroupItem(const Group *group)
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

void RosterModel::talkableDataChanged(QString id, QVariant data, TalkableDataRole role)
{
	RosterIndex *index = findRosterIndexById(id);
	if ( !index )
		return;

	index->setData(role, data);
	QModelIndex changed_index = modelIndexByRosterIndex(index);
	dataChanged(changed_index, changed_index);
}

void RosterModel::categoryDataChanged(int index, QVariant data, TalkableDataRole role)
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

void RosterModel::onDoubleClicked(const QModelIndex &index)
{
	ChatDlgManager *chat_mgr = ChatDlgManager::instance();

	QModelIndex real_index = proxy_ ? proxy_->mapToSource(index) : index;

	RosterIndex *roster_index = rosterIndexByModelIndex(real_index);

	RosterIndexType type = roster_index->type();
	if ( type == RIT_Contact )
	{
		QString id = roster_index->data(TDR_Id).toString();
        if ( Roster::instance()->contact(id) )
            chat_mgr->openFriendChatDlg(id);
	}
    else if ( type == RIT_Sess )
    {
        sigDoubleClicked(roster_index);
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

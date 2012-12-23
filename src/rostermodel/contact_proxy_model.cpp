#include "contact_proxy_model.h"

#include <QModelIndex>

#include "rostermodel/roster_index.h"
#include "utils/contact_status.h"
#include "roster/roster.h"
#include "friendsearcher.h"

ContactProxyModel::ContactProxyModel(QObject *parent) :
	QSortFilterProxyModel(parent),
	searcher_(NULL),
	is_searching_(false)
{
	setDynamicSortFilter(true);
}

ContactProxyModel::~ContactProxyModel()
{
	if ( searcher_ )
		delete searcher_;

	searcher_ = NULL;	
}


bool ContactProxyModel::lessThan(const QModelIndex & left, const QModelIndex & right) const
{
	RosterIndex *lindex = (RosterIndex *)left.internalPointer();
	RosterIndex *rindex = (RosterIndex *)right.internalPointer();

	if ( lindex->type() == RIT_Category )
	{
		return !(lindex->data(TDR_CategoryIndex).toInt() < rindex->data(TDR_CategoryIndex).toInt());
	}

	ContactStatus lstatus = lindex->data(TDR_Status).value<ContactStatus>();
	ContactStatus rstatus = rindex->data(TDR_Status).value<ContactStatus>();

	return !(lstatus < rstatus);
}

bool ContactProxyModel::filterAcceptsRow(int row, const QModelIndex &parent) const
{
	if ( !is_searching_ )
		return true;

	QModelIndex child = sourceModel()->index(row, 0, parent);
	if ( !child.isValid() )
		return false;

	RosterIndex *roster_child = (RosterIndex *)child.internalPointer();

	if ( roster_child->type() == RIT_Category )
	{
		QString index = QString::number(roster_child->data(TDR_CategoryIndex).toInt());
		if ( search_result_.contains(index) )
		{
			return true;
		}
	}
	else
	{
		if ( search_result_.contains(roster_child->data(TDR_Id).toString()) )
			return true;
	}

	return false;
}

void ContactProxyModel::setFilter(FriendSearcher *searcher)
{
	searcher_ = searcher;
}

void ContactProxyModel::onSearch(const QString &str)
{
	search_result_.clear();

	if ( !str.isEmpty() )
	{
		is_searching_ = true;
		searcher_->search(str, search_result_);

		//contact's category also should be shown
		foreach ( const QString &id, search_result_ )
		{
			Contact *contact = Roster::instance()->contact(id);
			QString cat_index = QString::number(contact->category()->index());
			if ( !search_result_.contains(cat_index) )
				search_result_.append(cat_index);
		}
		invalidateFilter();
	}
	else
	{
		is_searching_ = false;
		invalidateFilter();
	}
}

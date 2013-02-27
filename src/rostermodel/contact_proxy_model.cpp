#include "contact_proxy_model.h"

#include <QModelIndex>

#include "rostermodel/roster_index.h"
#include "utils/contact_status.h"

ContactProxyModel::ContactProxyModel(QObject *parent) :
	QSortFilterProxyModel(parent)
{
	setDynamicSortFilter(true);
}

ContactProxyModel::~ContactProxyModel()
{
}

bool ContactProxyModel::lessThan(const QModelIndex & left, const QModelIndex & right) const
{
	RosterIndex *lindex = (RosterIndex *)left.internalPointer();
	RosterIndex *rindex = (RosterIndex *)right.internalPointer();

	if ( lindex->type() == RIT_Category )
	{
        CategoryIndex *cl = (CategoryIndex *)lindex;
        CategoryIndex *cr = (CategoryIndex *)rindex;
		return !(cl->index() < cr->index());
	}

	ContactStatus lstatus = lindex->status();
	ContactStatus rstatus = rindex->status();

	return !(lstatus < rstatus);
}

bool ContactProxyModel::filterAcceptsRow(int row, const QModelIndex &parent) const
{
	if ( filter_.isEmpty() )
		return true;

	QModelIndex child = sourceModel()->index(row, 0, parent);
	if ( !child.isValid() )
		return false;

	RosterIndex *roster_child = (RosterIndex *)child.internalPointer();

	if ( roster_child->type() == RIT_Category )
	{
        CategoryIndex *cat_idx = (CategoryIndex *)roster_child;
		QString index = QString::number(cat_idx->index());
		if ( filter_.contains(index) )
		{
			return true;
		}
	}
	else
	{
		if ( filter_.contains(roster_child->id()) )
			return true;
	}

	return false;
}

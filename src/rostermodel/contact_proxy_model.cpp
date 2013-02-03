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
		return !(lindex->data(TDR_CategoryIndex).toInt() < rindex->data(TDR_CategoryIndex).toInt());
	}

	ContactStatus lstatus = lindex->data(TDR_Status).value<ContactStatus>();
	ContactStatus rstatus = rindex->data(TDR_Status).value<ContactStatus>();

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
		QString index = QString::number(roster_child->data(TDR_CategoryIndex).toInt());
		if ( filter_.contains(index) )
		{
			return true;
		}
	}
	else
	{
		if ( filter_.contains(roster_child->data(TDR_Id).toString()) )
			return true;
	}

	return false;
}

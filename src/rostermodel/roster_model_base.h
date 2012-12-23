#ifndef ROSTER_MODEL_BASE_H
#define ROSTER_MODEL_BASE_H

#include <QAbstractItemModel>
#include <QAbstractProxyModel>

#include "utils/contact_status.h"

class RosterIndex;

class __RosterModelBase : public QAbstractItemModel
{
public:
	__RosterModelBase(QObject *parent = NULL);
	virtual ~__RosterModelBase();

	virtual QModelIndex parent(const QModelIndex &child) const;

	virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
	virtual QVariant data(const QModelIndex &index, int role) const;

	virtual int rowCount(const QModelIndex &parent) const;
	virtual int columnCount(const QModelIndex &parent) const;

	void setProxyModel(QAbstractProxyModel *proxy);
	QAbstractProxyModel *proxyModel() const;

protected:
	virtual RosterIndex *rosterIndexByModelIndex(const QModelIndex &index) const;
	virtual QModelIndex modelIndexByRosterIndex(const RosterIndex *index) const;

protected:
	RosterIndex *root_;
	QAbstractProxyModel *proxy_;
};

#endif //ROSTER_MODEL_BASE_H

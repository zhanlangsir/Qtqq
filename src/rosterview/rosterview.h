#ifndef ROSTERVIEW_H
#define ROSTERVIEW_H

#include <QTreeView>
#include <QAbstractProxyModel>
#include <QModelIndex>

#include "rostermodel/roster_model_base.h"

class QContextMenuEvent;

class Menu;
class RosterIndex;

class RosterView : public QTreeView
{
    Q_OBJECT

signals:
    void indexContextMenu(RosterIndex *idx, Menu *menu);

public:
    RosterView(QWidget *parent = 0);

    QModelIndex currentSelectedIndex();

    void setRosterModel(__RosterModelBase *model);
    void setProxyModel(QAbstractProxyModel *proxy);

    QModelIndex mapToSource(QModelIndex index) const;

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private:
    __RosterModelBase *model_;
    QAbstractProxyModel *proxy_;
};

inline 
QModelIndex RosterView::currentSelectedIndex()
{
    QModelIndex curr_idx = currentIndex();
    if ( curr_idx.isValid() )
        return mapToSource(curr_idx);

    return QModelIndex();
}

inline 
void RosterView::setRosterModel(__RosterModelBase *model)
{
    if ( !proxy_ )
        QTreeView::setModel(model);

    model_ = model;
}

inline
void RosterView::setProxyModel(QAbstractProxyModel *proxy)
{
    proxy_ = proxy;
    QTreeView::setModel(proxy);
}

inline
QModelIndex RosterView::mapToSource(QModelIndex index) const
{
    if ( proxy_ )
        return proxy_->mapToSource(index);

    return index;
}

#endif //ROSTERVIEW_H

#include "rosterview.h"

#include <assert.h>

#include <QContextMenuEvent>

#include "rostermodel/roster_index.h"
#include "utils/menu.h"

RosterView::RosterView(QWidget *parent) :
    QTreeView(parent),
    proxy_(NULL)
{
}

void RosterView::contextMenuEvent(QContextMenuEvent *event)
{
    QModelIndex index = mapToSource(indexAt(event->pos()));

    if ( !index.isValid() )
        return;

    RosterIndex *r_index = model_->rosterIndexByModelIndex(index);

    assert(r_index);

    Menu *menu = new Menu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose, true);
    emit indexContextMenu(r_index, menu);

    if ( !menu->isEmpty() )
        menu->popup(event->globalPos());
    else
        delete menu;
}

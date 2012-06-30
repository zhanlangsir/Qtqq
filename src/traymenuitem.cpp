#include "traymenuitem.h"

#undef signals
#include <gtk/gtk.h>

static void trayMenuItemClicked(GtkMenuItem *item, gpointer source);

TrayMenuItem::TrayMenuItem(QString label, QObject *parent) :
    QObject(parent)
{
    item_ = gtk_menu_item_new_with_label(label.toLatin1().data());
    g_signal_connect (G_OBJECT (item_), "activate", G_CALLBACK (trayMenuItemClicked), this);
}

static void trayMenuItemClicked(GtkMenuItem *item, gpointer source)
{
    TrayMenuItem *menu_item = (TrayMenuItem *)source;
    menu_item->emitTriggered();
}

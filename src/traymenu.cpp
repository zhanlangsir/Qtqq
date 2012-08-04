#include "traymenu.h"

#include "traymenuitem.h"

#undef signals
#include <gtk/gtk.h>

TrayMenu::TrayMenu(QObject *parent) : QObject(parent)
{
    menu_ = gtk_menu_new();
    gtk_widget_show_all(menu_);
}

void TrayMenu::appendMenuItem(TrayMenuItem *item)
{
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_), item->item_);
    gtk_widget_show(item->item_);
}

void TrayMenu::insertMenuItem(TrayMenuItem *item, int idx)
{
}

void TrayMenu::popup() const
{
    gtk_menu_popup(GTK_MENU(menu_), NULL, NULL, NULL, NULL, 3, gtk_get_current_event_time());
}

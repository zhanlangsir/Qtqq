#include "menu.h"

Menu::Menu(QWidget *parent) :
    QMenu(parent)
{
}

void Menu::addPluginAction(QAction *action)
{
    insertAction(plugin_sperator_, action); 
}

void Menu::addPluginSperator()
{
    plugin_sperator_ = addSeparator();
}

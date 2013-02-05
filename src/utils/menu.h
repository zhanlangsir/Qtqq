#ifndef MENU_H
#define MENU_H

#include <QMenu>

class Menu : public QMenu
{
public:
    Menu(QWidget *parent = NULL);

    void addPluginAction(QAction *action);
    void addPluginSperator();

private:
    QAction *plugin_sperator_;
};

#endif //MENU_H

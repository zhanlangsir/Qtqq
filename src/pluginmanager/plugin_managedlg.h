#ifndef PLUGIN_MANAGERDLG_H
#define PLUGIN_MANAGERDLG_H

#include <QDialog>
#include <QDomDocument>
#include <QDomElement>

#include "ui_pluginmanagedlg.h"

namespace Ui
{
    class PluginWindow;
}

class PluginManager;

class PluginManageDlg : public QDialog
{
    Q_OBJECT
public:
    PluginManageDlg(PluginManager *plugin_mgr, QDomDocument plugin_config, QWidget *parent = NULL);

private slots:
    void onConfigureBtnClicked();
    void onAcceptBtnClicked();
    void onCurrentPluginChanged(QTableWidgetItem *current, QTableWidgetItem * /*previous*/);
    void onHomePageLinkActivated(const QString &link);

private:
    void updatePluginList(QDomDocument plugin_config);

private:
    Ui::PluginWindow ui;

    PluginManager *plugin_mgr_;

    QMap<QTableWidgetItem *, QDomElement> plugins_;
};

#endif //PLUGIN_MANAGERDLG_H

#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <QDomDocument>
#include <QDir>
#include <QString>
#include <QMap>
#include <QObject>
#include <QPluginLoader>

#include "interfaces/iplugin.h"
#include "qqglobal.h"

class PluginManageDlg;

class PluginManager : public QObject
{
    Q_OBJECT
public:
    ~PluginManager();

    static PluginManager *instance()
    {
        if ( !instance_ )
            instance_ = new PluginManager();

        return instance_;
    }

    IPlugin *plugin(QUuid id)
    { return plugins_.value(id, NULL); }

    void loadSettings();
    void loadPlugins();

    void enablePlugin(QUuid id);
    void disablePlugin(QUuid id);

private slots:
    void openPluginManageDlg();
    void onPluginManageDlgAccept();

private:
    void createMenuAction();
    void savePluginInfo(const QString &file_name, IPlugin *plugin);
    void savePluginConfig();
    void removeUnexistPluginInfo(const QStringList &plugin_list);

private:
    QDomDocument plugins_config_;
    QMap<QUuid, IPlugin *> plugins_;

    PluginManageDlg *pluginmanager_dlg_;

private:
    PluginManager();
    PluginManager(const PluginManager &);
    PluginManager& operator=(const PluginManager &);

    static PluginManager *instance_;
};

#endif //PLUGIN_MANAGER_H

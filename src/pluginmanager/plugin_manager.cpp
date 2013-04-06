#include "plugin_manager.h"

#include <QFile>
#include <QAction>
#include <QDebug>

#include "qtqq.h"
#include "mainwindow.h"
#include "utils/menu.h"
#include "pluginmanager/plugin_managedlg.h"

#define PLUGIN_CONFIG_FILE "plugins.xml"

PluginManager *PluginManager::instance_ = NULL;

PluginManager::PluginManager() :
    pluginmanager_dlg_(NULL)
{
}

PluginManager::~PluginManager()
{
    savePluginConfig();
}

void PluginManager::loadSettings()
{
    QDir plugin_dir(QQGlobal::configDir());

    QFile file(plugin_dir.absoluteFilePath(PLUGIN_CONFIG_FILE));
    if ( file.exists() && file.open(QIODevice::ReadOnly) )
        plugins_config_.setContent(&file, true);
    file.close();

	if (plugins_config_.documentElement().tagName() != "plugins" )
	{
		plugins_config_.clear();
		plugins_config_.appendChild(plugins_config_.createElement("plugins"));
	}
}

void PluginManager::loadPlugins()
{
    qDebug() << "Begin load plugins" << endl;
    QDir plugins_dir(QQGlobal::pluginDir());
    removeUnexistPluginInfo(plugins_dir.entryList());

    foreach ( QString file_name, plugins_dir.entryList(QDir::Files) )
    {
        QPluginLoader loader(plugins_dir.absoluteFilePath(file_name));
        if ( loader.load() )
        {
            IPlugin *plugin = qobject_cast<IPlugin *>(loader.instance());
            if (plugin) 
            {
                plugin->load();
                savePluginInfo(file_name, plugin);
                plugins_.insert(plugin->uuid(), plugin);
            }
            else
            {
                qDebug() << "Wrong plugin interface!\n" 
                         << "File name: " << file_name << endl;
            }
        }
        else
        {
            qDebug() << "plugin loaded failed!\n"
                     << "File name: " << file_name << '\n'
                     << "Error :" << loader.errorString() << endl;
        }
    }

    createMenuAction();
}

void PluginManager::savePluginInfo(const QString &file_name, IPlugin *plugin)
{
    PluginInfo info;
    plugin->pluginInfo(&info);

    QDomElement plugin_elem = plugins_config_.documentElement().firstChildElement(file_name);
    if ( plugin_elem.isNull() )
    {
        plugin_elem = plugins_config_.createElement(file_name).toElement();
       plugins_config_.firstChildElement("plugins").appendChild(plugin_elem); 
    }
    plugin_elem.setAttribute("uuid", plugin->uuid().toString());

    QDomElement name_elem = plugin_elem.firstChildElement("name");
    if ( name_elem.isNull() )
    {
        name_elem = plugin_elem.appendChild(plugins_config_.createElement("name")).toElement();
        name_elem.appendChild(plugins_config_.createTextNode(info.name));
    }
    else
        name_elem.firstChild().toCharacterData().setData(info.name);

	QDomElement desc_elem = plugin_elem.firstChildElement("description");
	if (desc_elem.isNull())
	{
		desc_elem = plugin_elem.appendChild(plugins_config_.createElement("description")).toElement();
		desc_elem.appendChild(plugins_config_.createTextNode(info.description));
	}
	else
		desc_elem.firstChild().toCharacterData().setData(info.description);

	QDomElement version_elem = plugin_elem.firstChildElement("version");
	if (version_elem.isNull())
	{
		version_elem = plugin_elem.appendChild(plugins_config_.createElement("version")).toElement();
		version_elem.appendChild(plugins_config_.createTextNode(info.version));
	}
	else
		version_elem.firstChild().toCharacterData().setData(info.version);

    qDebug() << "Loaded plugin information:\n" << plugins_config_.toString() << endl;
}

void PluginManager::removeUnexistPluginInfo(const QStringList &plugin_list)
{
    QDomElement plugin_elem = plugins_config_.documentElement().firstChildElement();

    while ( !plugin_elem.isNull() )
    {
        if (!plugin_list.contains(plugin_elem.tagName()))
        {
            QDomElement old_elem = plugin_elem;
            plugin_elem = plugin_elem.nextSiblingElement();
            old_elem.parentNode().removeChild(old_elem);
        }
        else
        {
            plugin_elem = plugin_elem.nextSiblingElement();
        }
    }
}

void PluginManager::createMenuAction()
{
    MainWindow *main_win = Qtqq::instance()->mainWindow();

    QAction *act_plugin = new QAction(tr("Plugin"), main_win->mainMenu());
    act_plugin->setCheckable(false);
    connect(act_plugin, SIGNAL(triggered()), this, SLOT(openPluginManageDlg()));
    main_win->mainMenu()->addPluginAction(act_plugin);
}

void PluginManager::openPluginManageDlg()
{
    if ( !pluginmanager_dlg_ )
    {
        pluginmanager_dlg_ = new PluginManageDlg(this, plugins_config_);
        connect(pluginmanager_dlg_, SIGNAL(accepted()), this, SLOT(onPluginManageDlgAccept()));
    }

    pluginmanager_dlg_->show();
}

void PluginManager::onPluginManageDlgAccept()
{
    savePluginConfig();
}

void PluginManager::savePluginConfig()
{
	if (!plugins_config_.documentElement().isNull())
	{
		QDir config_dir(QQGlobal::configDir());
		QFile file(config_dir.absoluteFilePath(PLUGIN_CONFIG_FILE ));
		if (file.open(QFile::WriteOnly|QFile::Truncate))
		{
			file.write(plugins_config_.toString(4).toUtf8());
			file.flush();
			file.close();
		}
	}
}

void PluginManager::enablePlugin(QUuid id)
{
    if ( id.isNull() )
        return;

    IPlugin *plugin = plugins_.value(id, NULL);
    if ( plugin )
    {
        plugin->load();
    }
}

void PluginManager::disablePlugin(QUuid id)
{
    if ( id.isNull() )
        return;

    IPlugin *plugin = plugins_.value(id, NULL);
    if ( plugin )
    {
        plugin->unload();
    }
}

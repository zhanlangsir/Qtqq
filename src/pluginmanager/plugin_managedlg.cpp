#include "plugin_managedlg.h"

#include <assert.h>

#include <QDesktopServices>
#include <QTableWidgetItem>
#include <QDebug>

#include "pluginmanager/plugin_manager.h"

#define PLUGIN_CONFIG_FILE "plugins.xml"

PluginManageDlg::PluginManageDlg(PluginManager *plugin_mgr, QDomDocument plugin_config, QWidget *parent) :
    QDialog(parent),
    plugin_mgr_(plugin_mgr)
{
    ui.setupUi(this);

    connect(ui.configure_btn, SIGNAL(clicked()), this, SLOT(onConfigureBtnClicked()));
    connect(ui.close_btn, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui.accept_btn, SIGNAL(clicked()), this, SLOT(onAcceptBtnClicked()));

	connect(ui.tw_plugins,SIGNAL(currentItemChanged(QTableWidgetItem *, QTableWidgetItem *)),SLOT(onCurrentPluginChanged(QTableWidgetItem *, QTableWidgetItem *)));
	connect(ui.homepage_label, SIGNAL(linkActivated(const QString &)),SLOT(onHomePageLinkActivated(const QString &)));

    updatePluginList(plugin_config);
}

void PluginManageDlg::onCurrentPluginChanged(QTableWidgetItem *current, QTableWidgetItem * /*previous*/)
{
    QDomElement plugin_elem  = plugins_.value(current); 
    IPlugin *plugin = plugin_mgr_->plugin(QUuid(plugin_elem.attribute("uuid", "")));
    assert(plugin);
    PluginInfo info;
    plugin->pluginInfo(&info);

    QString name = plugin_elem.firstChildElement("name").text().isEmpty() ? plugin_elem.tagName() : plugin_elem.firstChildElement("name").text();
    ui.plugin_name_label->setText(QString("<b>%1</b> %2").arg(name).arg(plugin_elem.firstChildElement("version").text()));
    ui.description_label->setText(plugin_elem.firstChildElement("description").text());
    ui.email_label->setText(info.email);
    ui.author_label->setText(info.author);
    ui.homepage_label->setText(QString("<a href='%1'>%2</a>").arg(info.home_page.toString()).arg(info.home_page.toString()));
}

void PluginManageDlg::onHomePageLinkActivated(const QString &link)
{
	QDesktopServices::openUrl(link);
}

void PluginManageDlg::updatePluginList(QDomDocument plugin_config)
{
	QDomElement plugin_elem = plugin_config.documentElement().firstChildElement();
	while ( !plugin_elem.isNull() )
	{
		QString name = plugin_elem.firstChildElement("name").text().isEmpty() ? plugin_elem.tagName() : plugin_elem.firstChildElement("name").text();
		QTableWidgetItem *name_item = new QTableWidgetItem(name);
		if ( plugin_elem.attribute("enabled","true")=="true" )
		{
			if (plugin_mgr_->plugin(plugin_elem.attribute("uuid"))==NULL)
				name_item->setForeground(Qt::red);
			name_item->setCheckState(Qt::Checked);
		}
		else
		{
			name_item->setForeground(Qt::gray);
			name_item->setCheckState(Qt::Unchecked);
		}
		name_item->setCheckState(plugin_elem.attribute("enabled","true")=="true" ? Qt::Checked : Qt::Unchecked);


		ui.tw_plugins->insertRow(ui.tw_plugins->rowCount());
		ui.tw_plugins->setItem(ui.tw_plugins->rowCount()-1, 0, name_item);
		
		plugins_.insert(name_item,plugin_elem);
		plugin_elem = plugin_elem.nextSiblingElement();
	}

	ui.tw_plugins->sortItems(0, Qt::AscendingOrder);
}

void PluginManageDlg::onConfigureBtnClicked()
{
    
}

void PluginManageDlg::onAcceptBtnClicked()
{
    QMap<QTableWidgetItem *, QDomElement>::const_iterator itor = plugins_.begin();

    while ( itor != plugins_.end() )
    {
        QDomElement plugin_elem = itor.value();
        if ( itor.key()->checkState() == Qt::Checked && plugin_elem.attribute("enabled", "true") == "false" )
        {
            plugin_mgr_->enablePlugin(QUuid(plugin_elem.attribute("uuid", "")));
            plugin_elem.removeAttribute("enabled");
        }
        else if (  itor.key()->checkState() == Qt::Unchecked && plugin_elem.attribute("enabled", "true") == "true" )
        {
            plugin_mgr_->disablePlugin(QUuid(plugin_elem.attribute("uuid", "")));
            plugin_elem.setAttribute("enabled", "false");
        }

        ++itor;
    }

    accept();
}

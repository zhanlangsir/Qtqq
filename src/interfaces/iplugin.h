#ifndef IPLUGIN_H
#define IPLUGIN_H

#include <QUuid>
#include <QUrl>
#include <QtPlugin>
#include <QWidget>

struct PluginInfo
{
	QString name;
	QString version;
	QString author;
    QString email;
	QUrl home_page;
	QString description;
};

class IPlugin
{
public:
    virtual QUuid uuid() const = 0;

    virtual void pluginInfo(PluginInfo *plugin_info) = 0;

    virtual QWidget *configWidget(QWidget *parent) = 0;

    virtual void load() = 0;
    virtual void unload() = 0;
};

Q_DECLARE_INTERFACE(IPlugin, "Qtqq.IPlugin/1.0")

#endif //IPLUGIN_H

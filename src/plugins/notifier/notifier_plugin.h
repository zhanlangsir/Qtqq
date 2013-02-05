#ifndef NOTIFIER_PLUGIN_H
#define NOTIFIER_PLUGIN_H

#include <QObject>

#include "interfaces/iplugin.h"
#include "core/qqmsg.h"

#define NOTIFIERPLUGIN_UUID "{49f286f0-8e35-45da-8c8a-10984123409a}"

class NotifierPlugin : 
   public QObject,
   public IPlugin
{
    Q_OBJECT
    Q_INTERFACES(IPlugin)
public:
    virtual QUuid uuid() const
    { return NOTIFIERPLUGIN_UUID; }
    virtual void pluginInfo(PluginInfo *plugin_info);
    virtual QWidget *configWidget(QWidget *parent);

    virtual void load();
    virtual void unload();

private slots:
    void onNewChatMsg(ShareQQMsgPtr msg);
};

#endif //NOTIFIER_PLUGIN_H

#include "notifier_plugin.h"

#include "plugins/notifier/notifywidget.h"
#include "notification_manager/notification_manager.h"
#include "roster/roster.h"
#include "strangermanager/stranger_manager.h"
#include "core/talkable.h"

void NotifierPlugin::pluginInfo(PluginInfo *plugin_info)
{
    plugin_info->name = tr("Notification");
    plugin_info->description = tr("Notification, alert from vacuum");
    plugin_info->version = "1.0";
    plugin_info->author = "zhanlang";
    plugin_info->email = "zhanlangsir@gmail.com";
    plugin_info->home_page = "http://www.aitilife.com/qtqq";
}

void NotifierPlugin::load()
{
    NotificationManager *mgr = NotificationManager::instance();    
    connect(mgr, SIGNAL(newChatMsg(ShareQQMsgPtr)), this, SLOT(onNewChatMsg(ShareQQMsgPtr)));
}

void NotifierPlugin::unload()
{
    NotificationManager *mgr = NotificationManager::instance();    
    disconnect(mgr, SIGNAL(newChatMsg(ShareQQMsgPtr)), this, SLOT(onNewChatMsg(ShareQQMsgPtr)));
}

void NotifierPlugin::onNewChatMsg(ShareQQMsgPtr msg)
{
    Notification notification;
    Talkable *talkable = NULL;
    QString sender_id;
    if ( msg->type() == QQMsg::kSess )
    {
        sender_id = msg->sendUin();
        talkable = StrangerManager::instance()->stranger(sender_id);
    }
    else
    {
        sender_id = msg->talkTo();
        talkable = Roster::instance()->talkable(sender_id);
        if ( !talkable )
            talkable = StrangerManager::instance()->stranger(sender_id);
    }

    QString sender_name = talkable ?  talkable->markname() : sender_id;
    notification.title = sender_name + tr(" has new message");  
    notification.icon = talkable ? talkable->avatar() : QPixmap();
    notification.content = msg->msg();
    notification.ms_timeout = 5000;

    switch ( msg->type() )
    {
        case QQMsg::kFriend:
            notification.type = NFT_Chat;
            break;
        case QQMsg::kGroup:
            notification.type = NFT_GroupChat;
            break;
        case QQMsg::kSess:
            notification.type = NFT_SessChat;
            break;
    }
    NotifyWidget *w = new NotifyWidget(notification);
    w->appear();
}

QWidget *NotifierPlugin::configWidget(QWidget *parent)
{
    return NULL;
}

Q_EXPORT_PLUGIN2(notifierplugin, NotifierPlugin)

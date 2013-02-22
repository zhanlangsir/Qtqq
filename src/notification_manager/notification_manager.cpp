#include "notification_manager.h"

#include "chatwidget/chatdlg_manager.h"
#include "msgprocessor/msg_processor.h"
#include  "chatwidget/tabwindow.h"

NotificationManager *NotificationManager::instance_ = NULL;

NotificationManager::NotificationManager()
{
    connect(MsgProcessor::instance(), SIGNAL(newChatMsg(ShareQQMsgPtr)), this, SLOT(onNewChatMsg(ShareQQMsgPtr)));
}

void NotificationManager::onNewChatMsg(ShareQQMsgPtr msg)
{
    if ( !ChatDlgManager::instance()->isOpening(msg->talkTo())  || ChatDlgManager::instance()->tabWin()->isMinimized() )
    {
        emit newChatMsg(msg);
    }
}

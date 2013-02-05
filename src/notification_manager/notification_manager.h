#ifndef NOTIFICATION_MANAGER_H
#define NOTIFICATION_MANAGER_H

#include <QObject>
//#include <QVariant>

#include "core/qqmsg.h"

/* enum NotificationType  */
/* { */
/*     NFT_Chat, */
/*     NFT_GroupChat, */
/*     NFT_SessChat, */
/*     NFT_StatusChanged */
/* }; */

/* struct Notification */
/* { */
/*     NotificationType type; */
/*     QString title; */
/*     QString content; */
/*  */
/*     QPixmap icon; */
/*     QVariant data; */
/* }; */

class NotificationManager : public QObject
{
    Q_OBJECT
signals:
    void newChatMsg(ShareQQMsgPtr msg);

public:
    static NotificationManager *instance()
    {
        if ( !instance_ )
            instance_ = new NotificationManager();

        return instance_;
    }

private slots:
    void onNewChatMsg(ShareQQMsgPtr msg);

private:
    NotificationManager();
    NotificationManager(const NotificationManager &);
    NotificationManager &operator=(const NotificationManager &);

    static NotificationManager *instance_;
};

#endif //NOTIFICATION_MANAGER_H

#ifndef QTQQ_CORE_TYPES_H
#define QTQQ_CORE_TYPES_H

#include <QString>
#include <QMetaType>

#include "qqchatitem.h"

enum FriendStatus {kCallMe=60, kOnline=10, kAway=30, kBusy=50, kSilent=70, kHidden=40, kOffline=100};
enum ClientType{kPc=1, kPhone=21, kIphone=24, kWeb=41, kIpad};

Q_DECLARE_METATYPE(FriendStatus)

struct FileInfo
{
    int size;
    QString name;
    QString local_path;
    QString network_path;
    QQChatItem::ChatItemType type;
};

struct LoginInfo
{
    QString id;
    QString name;
    FriendStatus status;
    QString avatar_path;
};

#endif //QTQQ_CORE_TYPES_H

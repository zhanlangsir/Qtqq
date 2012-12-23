#ifndef QTQQ_CORE_TYPES_H
#define QTQQ_CORE_TYPES_H

#include <QString>

#include "core/qqchatitem.h"
#include "utils/contact_status.h"

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
    ContactStatus status;
    QString avatar_path;
};

#endif //QTQQ_CORE_TYPES_H

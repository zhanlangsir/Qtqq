#ifndef MSGSENDER_H
#define MSGSENDER_H

#include <QVector>
#include <QString>

#include "core/qqchatitem.h"

class Contact;
class Group;

class MsgSender
{
public:
    static QString msgToJson(Contact *to, const QVector<QQChatItem> &items);
    static QString groupMsgToJson(Group *to, const QVector<QQChatItem> &items);

private:
    static int msg_id_;
};

#endif //MSGSENDER_H

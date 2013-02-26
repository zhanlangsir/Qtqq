#ifndef MSGSENDER_H
#define MSGSENDER_H

#include <QHttp>
#include <QHttpRequestHeader>
#include <QVector>
#include <QString>
#include <QObject>
#include <QMap>

#include "core/qqchatitem.h"

class Contact;
class Group;

namespace Protocol
{
    class MsgSender;
};

class Protocol::MsgSender
{
public:
    QString msgToJson(Contact *to, const QVector<QQChatItem> &items);
    QString groupMsgToJson(Group *to, const QVector<QQChatItem> &items);
    QString sessMsgToJson(Contact *to, Group *group, const QVector<QQChatItem> &msgs);

    QString getGroupSig(QString gid, QString to_id);

private:
    static int msg_id_;

    QMap<QString, QString> group_sigs_;
};

#endif //MSGSENDER_H

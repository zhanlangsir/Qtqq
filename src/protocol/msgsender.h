#ifndef MSGSENDER_H
#define MSGSENDER_H

#include <QHttp>
#include <QHttpRequestHeader>
#include <QVector>
#include <QString>
#include <QObject>
#include <QMap>

#include "core/qqchatitem.h"

namespace Protocol
{
    class MsgSender;
};

class Protocol::MsgSender
{
public:
    QString msgToJson(QString id, const QVector<QQChatItem> &items);
    QString groupMsgToJson(QString id, QString gocde, const QVector<QQChatItem> &items);
    QString sessMsgToJson(QString id, QString gid, const QVector<QQChatItem> &msgs);

    QString getGroupSig(QString gid, QString to_id);

private:
    static int msg_id_;

    QMap<QString, QString> group_sigs_;
};

#endif //MSGSENDER_H

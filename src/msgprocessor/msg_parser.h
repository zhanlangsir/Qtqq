#ifndef MSG_PARSER_H
#define MSG_PARSER_H

#include <QByteArray>

#include "core/qqmsg.h"

class MsgParser
{
public:
    static QQMsg *parse(const QByteArray &raw_data);

private:
    static QQMsg *createMsg(QString type, const Json::Value result);
    static QQMsg *createFriendMsg(const Json::Value &result) const;
    static QQMsg *createGroupMsg(const Json::Value &result) const;
    static QQMsg *createSessMsg(const Json::Value &result) const;
    static QQMsg *createBuddiesStatusChangeMsg(const Json::Value &result) const;
    static QQMsg *createSystemGroupMsg(const Json::Value &result) const;
    static QQMsg *createSystemMsg(const Json::Value &result) const;

};

#endif //MSG_PARSER_H

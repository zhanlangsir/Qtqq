/*
（群消息）group_messageg:
from_uin : gid(群id)
to_uin : 当前登陆用户uin
send_uin: 发送此消息的成员uin


(临时会话)sess_message:
from_uin : 发送此消息的成员uin
to_uin : 当前登陆用户uin
id : gid(群id)


（好友消息）message:
from_uin : from_uin
to_uin : 当前登陆用户uin
*/

#ifndef QTQQ_CORE_QQMSG_H
#define QTQQ_CORE_QQMSG_H

#include "types.h"

#include <QMetaType>
#include <QVector>
#include <QSharedPointer>
#include <QMetaType>

class QQMsg
{
public:
    enum MsgType{kGroup, kFriend, kBuddiesStatusChange, kSystem, kSystemG, kSess};
    virtual ~QQMsg() {}

public:
    MsgType type() const
    { return type_; }
    void set_type(MsgType type)
    {
        if (type_ != type)
            type_ = type;
    }

    virtual QString sendUin() const { return ""; }
    virtual QString talkTo() const { return ""; }

    virtual QString gCode() const {return ""; }
    virtual QString msg() const { return ""; }

    virtual long time() const { return 0; }

    virtual FriendStatus status() { return kOnline; }
    virtual ClientType client_type() { return kPc; }

protected:
    MsgType type_;
};

typedef QSharedPointer<QQMsg> ShareQQMsgPtr;
Q_DECLARE_METATYPE(ShareQQMsgPtr)

class QQChatMsg : public QQMsg
{
public:
    QString talkTo() const
    { return from_uin_; }
    QString sendUin() const
    { return from_uin_; }
    long time() const { return time_; }

    QString msg_id_;
    QString msg_id2_;
    QString from_uin_;
    QString to_uin_;
    long time_;
    int size_;
    QString color_;
    QString font_name_;
    bool b_;
    bool i_;
    bool u_;

    QVector<QQChatItem> msg_;
};

class QQSessChatMsg : public QQChatMsg
{
public:
    QString talkTo() const
    { return gid_; }
    QString sendUin() const
    { return from_uin_; }

    QString gid_;
};

class QQGroupChatMsg : public QQChatMsg
{
public:
    QString talkTo() const
    { return to_uin_; }
    QString sendUin() const
    { return send_uin_; }

    QString gCode() const {return group_code_; }

    QString group_code_;
    QString send_uin_;
    QString info_seq_;
};

class QQStatusChangeMsg : public QQMsg
{
public:
    QString sendUin() const
    { return uin_; }

    FriendStatus status() { return status_; }
    ClientType client_type() { return client_type_; }

    QString uin_;
    FriendStatus status_;
    ClientType client_type_;
};

class QQSystemMsg : public QQMsg
{
public:
    QString talkTo() const
    { return from_; }

    QString from_;
    QString aid_;
    QString msg_;
    QString account_;
    QString systemmsg_type_;
    FriendStatus status_;
};

class QQSystemGMsg : public QQMsg
{
public:
    QString sendUin() const
    { return request_uin_; }
    QString talkTo() const
    { return from_uin_; }
    QString msg() const
    { return msg_; }

    QString msg_id_;
    QString msg_id2_;
    QString from_uin_;
    QString to_uin_;
    QString sys_g_type_;
    QString gcode_;
    QString t_gcode_;
    QString request_uin_;
    QString t_request_uin_;
    QString msg_;
};

#endif //QTQQ_CORE_QQMSG_H

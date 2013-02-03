/*
（群消息）group_messageg:
from_uin : gid(群id)
to_uin : gid(群id)
send_uin: 发送此消息的成员uin


(临时会话)sess_message:
from_uin : 发送此消息的成员uin
to_uin : 当前登陆用户uin
id : gid(群id)


（好友消息）message:
from_uin : 发送人uin
to_uin : 当前登陆用户uin

(系统群消息)system_g_message
from_uin : gid
to_uin : 本登录号的qq号
request_uin : 请求者uin
*/

#ifndef QQMSG_H
#define QQMSG_H

#include <QMetaType>
#include <QVector>
#include <QSharedPointer>
#include <QMetaType>

#include "utils/contact_status.h"
#include "core/qqchatitem.h"
#include "core/talkable.h"

class QQMsg
{
public:
    enum MsgType{kGroup, kFriend, kBuddiesStatusChange, kSystem, kSystemG, kSess};

	QQMsg(MsgType type) : type_(type) {}
    virtual ~QQMsg() {}

public:
    MsgType type() const
    { return type_; }
    void set_type(MsgType type)
    {
        if (type_ != type)
            type_ = type;
    }

	//the sender uin
    virtual QString sendUin() const { return ""; }
	//the aid you talk to, should be a group or a friend
    virtual QString talkTo() const { return ""; }

    virtual QString gid() const { return ""; }
    virtual QString gCode() const {return ""; }

    virtual QString msg() const { return ""; }

    virtual long time() const { return 0; }

    virtual ContactStatus status() { return CS_Online; }
    virtual ContactClientType client_type() { return CCT_Pc; }

protected:
    MsgType type_;
};

typedef QSharedPointer<QQMsg> ShareQQMsgPtr;
Q_DECLARE_METATYPE(ShareQQMsgPtr)

class QQChatMsg : public QQMsg
{
public:
	QQChatMsg(MsgType type = QQMsg::kFriend) :
		QQMsg(type)
	{
	}

    virtual QString talkTo() const
    { return from_uin_; }

    virtual QString sendUin() const
    { return from_uin_; }

    virtual long time() const 
	{ return time_; }

    virtual QString msg() const 
	{
		QString msg_str;
		foreach ( QQChatItem msg, msgs_ )
		{
			msg_str.append(msg.content());
		}
	}

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

    QVector<QQChatItem> msgs_;
};

class QQSessChatMsg : public QQChatMsg
{
public:
	QQSessChatMsg(MsgType type = kSess) :
		QQChatMsg(type)
	{
	}

    virtual QString talkTo() const
    { return from_uin_; }
    virtual QString sendUin() const
    { return from_uin_; }

    virtual QString gid() const 
    { return gid_; }

    QString gid_;
};

class QQGroupChatMsg : public QQChatMsg
{
public:
	QQGroupChatMsg(MsgType type = kGroup) :
		QQChatMsg(type)
	{
	}

    QString talkTo() const
    { return from_uin_; }
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
	QQStatusChangeMsg(MsgType type = kBuddiesStatusChange) :
		QQMsg(type)
	{
	}

    QString sendUin() const
    { return uin_; }

    ContactStatus status() { return status_; }
    ContactClientType client_type() { return client_type_; }

    QString uin_;
    ContactStatus status_;
    ContactClientType client_type_;
};

class QQSystemMsg : public QQMsg
{
public:
	QQSystemMsg(MsgType type = kSystem) :
		QQMsg(type)
	{
	}

    QString talkTo() const
    { return from_; }
	QString sendUin() const
	{ return from_; }

    QString from_;
    QString aid_;
    QString msg_;
    QString account_;
    QString systemmsg_type_;
    ContactStatus status_;
};

class QQSystemGMsg : public QQMsg
{
public:
	QQSystemGMsg(MsgType type = kSystemG) :
		QQMsg(type)
	{
	}

    QString sendUin() const
    { return request_uin; }
    QString talkTo() const
    { return from_uin; }
    QString msg() const
    { return msg_; }

    QString msg_id_;
    QString msg_id2_;
    QString from_uin;
    QString to_uin;
    QString sys_g_type;
    QString gcode_;
    QString t_gcode_;
    QString request_uin;
    QString t_request_uin_;
    QString msg_;
};

#endif //QQMSG_H

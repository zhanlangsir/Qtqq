#pragma once

#include "types.h"

#include <QMetaType>
#include <QVector>

struct FontInfo
{
    int size_;
    QString color_;
    QString font_name_;
    bool b_;
    bool i_;
    bool u_;
};

class QQChatItem
{
public:
    enum ChatItemType {kQQFace, kFriendOffpic, kFriendCface, kGroupChatImg, kWord};
    QQChatItem(ChatItemType type, QString content) : type_(type), content_(content) {}
    QQChatItem() {}

    ChatItemType type() const
    { return type_; }
    void set_type(ChatItemType type)
    { type_ = type; }

    QString content() const
    { return content_; }
    void set_content(QString content)
    { content_ = content; }

private:
    ChatItemType type_;
    QString content_ ;
};

class QQMsg
{
public:
    enum MsgType{kGroup, kFriend, kBuddiesStatusChange, kSystem};

    MsgType type() const
    { return type_; }
    void set_type(MsgType type)
    { type_ = type; }

    virtual QString sendUin() const { return ""; }
    virtual QString talkTo() const { return ""; }

    virtual QString gCode() const {return ""; }

protected:
    MsgType type_;
};

class QQChatMsg : public QQMsg
{
public:
    QString talkTo() const
    { return from_uin_; }
    QString sendUin() const
    { return from_uin_; }
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

    QString uin_;
    FriendStatus status_;
    ClientType client_type_;
};

class QQSystemMsg : public QQMsg
{
public:
    QString from_;
    QString aid_;
    QString msg_;
};

class QQSystemGMsg : public QQMsg
{
public:

};

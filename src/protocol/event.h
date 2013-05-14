#ifndef EVENT_H
#define EVENT_H

#include <QString>
#include <QByteArray>

#include "protocol/request_jobs/img_type.h"
#include "core/talkable.h"

namespace Protocol
{
    enum EventType
    {
        ET_OnAvatarUpdate,
        ET_OnGroupMemberAvatarUpdate,
        ET_OnStrangerAvatarUpdate,
        ET_OnSlnUpdate,
        ET_OnGroupMemberListUpdate,
        ET_OnMsgSendDone,
        ET_OnImgSendDone,
        ET_OnImgLoadDone ,
        ET_OnStrangerInfoDone
    };

    class Event;
    class AvatarUpdateEvent;
    class GroupMemberAvatarUpdateEvent;
    class GroupMemberListUpdateEvent;
    class MsgSendDoneEvent;
    class ImgSendDoneEvent;
    class ImgLoadDoneEvent;
    class StrangerAvatarUpdateEvent;
    class StrangerInfoDoneEvent;
};

class Protocol::Event
{
public:
    Event(Protocol::EventType type, QString for_id, Talkable::TalkableType t_type, QByteArray data) :
        type_(type),
        for_id_(for_id),
        t_type_(t_type),
        data_(data)
    {
    }

    EventType type() const
    { return type_; }
    QByteArray data() const
    { return  data_; }
    QString forId() const
    { return for_id_; }
    Talkable::TalkableType forType() const
    { return t_type_; }

private:
    EventType type_;
    QString for_id_;
    Talkable::TalkableType t_type_;
    QByteArray data_;
};

class Protocol::AvatarUpdateEvent : public Protocol::Event
{
public:
    AvatarUpdateEvent(QString id, Talkable::TalkableType t_type, QByteArray data, EventType type = Protocol::ET_OnAvatarUpdate) :
        Event(type, id, t_type, data)
    {
    }
};

class Protocol::GroupMemberAvatarUpdateEvent : public Protocol::Event
{
public:
    GroupMemberAvatarUpdateEvent(QString id, QString gid, Talkable::TalkableType t_type, QByteArray data, EventType e_type = Protocol::ET_OnGroupMemberAvatarUpdate) :
        Event(e_type, id, t_type, data),
        gid_(gid)
    {
    }

    QString gid() const
    { return gid_; }

private:
    QString gid_;
};

class Protocol::StrangerAvatarUpdateEvent : public Protocol::Event
{
public:
    StrangerAvatarUpdateEvent(QString id, Talkable::TalkableType t_type, QByteArray data, EventType type = Protocol::ET_OnStrangerAvatarUpdate) :
        Event(type, id, t_type, data)
    {
    }
};

class Protocol::GroupMemberListUpdateEvent : public Protocol::Event
{
public:
    GroupMemberListUpdateEvent(QString id, Talkable::TalkableType t_type,  QByteArray data, Protocol::EventType type = Protocol::ET_OnGroupMemberListUpdate) :
        Event(type, id, t_type, data)
    {

    }
};

class Protocol::MsgSendDoneEvent : public Protocol::Event
{
public:
    MsgSendDoneEvent(QString id, Talkable::TalkableType t_type,  QByteArray data, Protocol::EventType type = Protocol::ET_OnMsgSendDone) :
        Event(type, id, t_type, data)
    {
    }
};

class Protocol::ImgSendDoneEvent : public Protocol::Event
{
public:
    ImgSendDoneEvent(QString id, Talkable::TalkableType t_type, bool is_success, QString file_path, QString img_id, Protocol::EventType type = Protocol::ET_OnImgSendDone) :
        Event(type, id, t_type, QByteArray()),
        is_success_(is_success),
        file_path_(file_path),
        img_id_(img_id)
    {
    }

    bool success() const
    { return is_success_; }

    QString imgId() const
    { return img_id_; }

    QString filePath() const
    { return file_path_; }

private:
    bool is_success_;
    QString file_path_;
    QString img_id_;
};

class Protocol::StrangerInfoDoneEvent : 
    public Event
{
public:
    StrangerInfoDoneEvent(QString id, Talkable::TalkableType t_type, QByteArray data, Protocol::EventType type = Protocol::ET_OnStrangerInfoDone) :
        Event(type, id, t_type, data)
    {
    }
};

class Protocol::ImgLoadDoneEvent : 
    public Event
{
public:
    ImgLoadDoneEvent(QString file_name, QString for_id, ImgType img_type, QByteArray data, Protocol::EventType type = Protocol::ET_OnImgLoadDone) :
        Event(type, for_id, (Talkable::TalkableType)-1, data),
        file_(file_name),
        img_type_(img_type)
    {
    }

    ImgType type() const
    { return img_type_; }
    QString file() const
    { return file_; }

private:
    QString file_;
    ImgType img_type_;
};

#endif //EVENT_H

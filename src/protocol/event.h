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
    Event(Protocol::EventType type, Talkable *event_for, QByteArray data) :
        type_(type),
        for_(event_for),
        data_(data)
    {
    }

    EventType type() const
    { return type_; }
    QByteArray data() const
    { return  data_; }
    Talkable *eventFor() const
    { return for_; }
    virtual QString forId() const
    { return for_->id(); }

private:
    EventType type_;
    Talkable *for_;
    QByteArray data_;
};

class Protocol::AvatarUpdateEvent : public Protocol::Event
{
public:
    AvatarUpdateEvent(Talkable *updateing, QByteArray data, EventType type = Protocol::ET_OnAvatarUpdate) :
        Event(type, updateing, data)
    {
    }
};

class Protocol::StrangerAvatarUpdateEvent : public Protocol::Event
{
public:
    StrangerAvatarUpdateEvent(Talkable *updateing, QByteArray data, EventType type = Protocol::ET_OnStrangerAvatarUpdate) :
        Event(type, updateing, data)
    {
    }
};

class Protocol::GroupMemberListUpdateEvent : public Protocol::Event
{
public:
    GroupMemberListUpdateEvent(Talkable *group,  QByteArray data, Protocol::EventType type = Protocol::ET_OnGroupMemberListUpdate) :
        Event(type, group, data)
    {

    }
};

class Protocol::MsgSendDoneEvent : public Protocol::Event
{
public:
    MsgSendDoneEvent(Talkable *_for,  QByteArray data, Protocol::EventType type = Protocol::ET_OnMsgSendDone) :
        Event(type, _for, data)
    {
    }
};

class Protocol::ImgSendDoneEvent : public Protocol::Event
{
public:
    ImgSendDoneEvent(Talkable *_for, bool is_success, QString file_path, QString img_id, Protocol::EventType type = Protocol::ET_OnImgSendDone) :
        Event(type, _for, QByteArray()),
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
    StrangerInfoDoneEvent(Talkable *_for, QByteArray data, Protocol::EventType type = Protocol::ET_OnStrangerInfoDone) :
        Event(type, _for, data)
    {
    }
};

class Protocol::ImgLoadDoneEvent : 
    public Event
{
public:
    ImgLoadDoneEvent(QString file_name, QString for_id, ImgType img_type, QByteArray data, Protocol::EventType type = Protocol::ET_OnImgLoadDone) :
        Event(type, NULL, data),
        file_(file_name),
        for_id_(for_id),
        img_type_(img_type)
    {
    }

    virtual QString forId() const
    {
        return for_id_;
    }

    ImgType type() const
    { return img_type_; }
    QString file() const
    { return file_; }

private:
    QString file_;
    QString for_id_;
    ImgType img_type_;
};

#endif //EVENT_H

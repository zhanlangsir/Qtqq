#ifndef EVENT_H
#define EVENT_H

#include <QString>
#include <QByteArray>

#include "core/talkable.h"

namespace Protocol
{
    enum EventType
    {
        ET_OnAvatarUpdate,
        ET_OnSlnUpdate,
        ET_OnGroupMemberListUpdate,
        ET_OnMsgSendDone,
        ET_OnImgSendDone
    };

    class Event;
    class AvatarUpdateEvent;
    class GroupMemberAvatarUpdateEvent;
    class GroupMemberListUpdateEvent;
    class MsgSendDoneEvent;
    class ImgSendDoneEvent;
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
#endif //EVENT_H

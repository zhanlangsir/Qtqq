#ifndef EVENT_CENTER_H
#define EVENT_CENTER_H

#include <QObject>
#include <QByteArray>
#include <QString>

#include "protocol/request_jobs/img_type.h"
#include "protocol/event.h"

namespace Protocol
{
    class EventCenter;
}

class Protocol::EventCenter : public QObject
{
    Q_OBJECT
signals:
    void eventTrigger(Protocol::Event *event);

public:
    static EventCenter *instance()
    {
        if ( !instance_ )
            instance_ = new EventCenter();

        return instance_;
    }

    Event *createGroupMemberListUpdateEvent(Group *group, QByteArray data)
    {
        return new GroupMemberListUpdateEvent(group, data);
    }
    Event *createAvatarUpdateEvent(Talkable *event_for, QByteArray data)
    {
        return new AvatarUpdateEvent(event_for, data);
    }
    Event *createMsgSendDoneEvent(Talkable *sender, QByteArray data)
    {
        return new MsgSendDoneEvent(sender, data);
    }
    Event *createImgSendDoneEvent(Talkable *sender, bool is_sucess, QString file_path, QString img_id)
    {
        return new ImgSendDoneEvent(sender, is_sucess, file_path, img_id);
    }
    Event *createStrangerInfoDoneEvent(Talkable *_for, QByteArray data)
    {
        return new StrangerInfoDoneEvent(_for, data);
    }
    Event *createStrangerAvatarUpdateEvent(Talkable *_for, QByteArray data)
    {
        return new StrangerAvatarUpdateEvent(_for, data);
    }
    Event *createImgLoadDoneEvent(QString file_name, QString for_id, ImgType type, QByteArray data)
    {
        return new ImgLoadDoneEvent(file_name, for_id, type, data);
    }

    void triggerEvent(Event *event);

private:
    EventCenter(){}
    EventCenter(const EventCenter &);
    EventCenter &operator=(const EventCenter &);

    static EventCenter *instance_;
};

#endif //EVENT_CENTER_H

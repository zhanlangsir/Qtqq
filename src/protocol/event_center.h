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

    Event *createGroupMemberListUpdateEvent(QString id, Talkable::TalkableType type, QByteArray data)
    {
        return new GroupMemberListUpdateEvent(id, type, data);
    }

    Event *createAvatarUpdateEvent(QString id, Talkable::TalkableType type, QByteArray data)
    {
        return new AvatarUpdateEvent(id, type, data);
    }
    Event *createGroupMemberAvatarUpdateEvent(QString id, QString gid, Talkable::TalkableType type, QByteArray data)
    {
        return new GroupMemberAvatarUpdateEvent(id, gid, type, data);
    }
    Event *createStrangerAvatarUpdateEvent(QString id, Talkable::TalkableType type, QByteArray data)
    {
        return new StrangerAvatarUpdateEvent(id, type, data);
    }

    Event *createMsgSendDoneEvent(QString id, Talkable::TalkableType type, QByteArray data)
    {
        return new MsgSendDoneEvent(id, type, data);
    }
    Event *createImgSendDoneEvent(QString id, Talkable::TalkableType type, bool is_sucess, QString file_path, QString img_id)
    {
        return new ImgSendDoneEvent(id, type, is_sucess, file_path, img_id);
    }
    Event *createStrangerInfoDoneEvent(QString id, Talkable::TalkableType type, QByteArray data)
    {
        return new StrangerInfoDoneEvent(id, type, data);
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

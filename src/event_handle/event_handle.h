#ifndef EVENT_HANDLE_H
#define EVENT_HANDLE_H

#include <QObject>
#include <QList>
#include <QByteArray>

#include "interfaces/isubject.h"
#include "protocol/event_center.h"

class Event;

class EventHandle : public QObject, public ISubject
{
    Q_OBJECT
public:
    static EventHandle *instance()
    {
        if ( !instance_ )
            instance_ = new EventHandle();

        return instance_;
    }

    virtual void registerObserver(Protocol::EventType type, IObserver *observer);
    virtual void removeObserver(Protocol::EventType type, IObserver *observer);

    void triggerEvent(Protocol::Event *e);

private slots:
    void onEventTrigger(Protocol::Event *e); 

private:
    struct SubscribeRecord
    {
        SubscribeRecord() {}
        SubscribeRecord(Protocol::EventType _type, IObserver *_observer) :
            subscribe_type(_type), 
            observer(_observer)
        {
        }

        Protocol::EventType subscribe_type;
        IObserver *observer;

        bool operator==(const SubscribeRecord &other)
        {
            return (other.subscribe_type == subscribe_type && other.observer == observer);
        }
    };

    SubscribeRecord findRecord(Protocol::EventType type, IObserver *observer);
    QList<SubscribeRecord> observers_;

private:
    EventHandle() {}
    EventHandle(const EventHandle &);
    EventHandle& operator=(const EventHandle &);

    static EventHandle *instance_;
};

#endif //EVENT_HANDLE_H

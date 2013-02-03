#include "event_handle.h"

#include "protocol/event.h"

EventHandle *EventHandle::instance_ = NULL;

void EventHandle::registerObserver(Protocol::EventType type, IObserver *observer)
{
    SubscribeRecord record;
    record.subscribe_type = type;
    record.observer = observer;

    if ( !observers_.contains(record) )
        observers_.append(record);
}

void EventHandle::removeObserver(Protocol::EventType type, IObserver *observer)
{
    observers_.removeOne(SubscribeRecord(type, observer));
}

EventHandle::SubscribeRecord EventHandle::findRecord(Protocol::EventType type, IObserver *observer)
{
    for ( int i = 0; i < observers_.count(); ++i )
    {
        SubscribeRecord record = observers_.at(i);
        if ( record.subscribe_type == type &&
             record.observer == observer )
        {
            return record;
        }
    }
}

void EventHandle::triggerEvent(Protocol::Event *e)
{
    foreach ( SubscribeRecord record, observers_ )
    {
        if ( record.subscribe_type == e->type() )
        {
            record.observer->onNotify(e);
        }
    }
}

void EventHandle::onEventTrigger(Protocol::Event *e)
{
    triggerEvent(e); 
}

#include "event_center.h"

#include <QDebug>

Protocol::EventCenter *Protocol::EventCenter::instance_ = NULL;

void Protocol::EventCenter::triggerEvent(Event *event)
{
    /* For Debug
    if ( event->type() != ET_OnAvatarUpdate )
    {
        qDebug() << "event: " << event->type() << "trigger!" << endl;
        qDebug() << "request: " << event->eventFor()->id() << endl;
        qDebug() << "event data: " << event->data() << "\n" << endl;
    }
    */

    emit eventTrigger(event);

    delete event;
    event = NULL;
}

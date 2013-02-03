#ifndef ISUBJECT_H
#define ISUBJECT_H

#include "interfaces/iobserver.h"
#include "protocol/event_center.h"

class ISubject
{
public:
    virtual void registerObserver(Protocol::EventType type, IObserver *observer) = 0;
    virtual void removeObserver(Protocol::EventType type, IObserver *observer) = 0;
};

#endif //ISUBJECT_H

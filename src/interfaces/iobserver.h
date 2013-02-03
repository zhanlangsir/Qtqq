#ifndef IOBSERVER_H
#define IOBSERVER_H

#include "protocol/event.h"

class IObserver
{
public:
    virtual void onNotify(Protocol::Event *event) = 0;
};

#endif //IOBSERVER_H

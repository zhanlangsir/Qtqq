#ifndef INOTIFIER_H
#define INOTIFIER_H

#include <QtPlugin>

class INotifier
{
public:
    virtual void notify() = 0;
}

Q_DECLARE_INTERFACE(IPlugin, "Qtqq.INotifier/1.0")

#endif //INOTIFIER_H

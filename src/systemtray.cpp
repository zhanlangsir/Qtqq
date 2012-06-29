#include "systemtray.h"

#include <QEvent>
#include <QDebug>

SystemTray::SystemTray(QObject *parent) : QSystemTrayIcon(parent)
{

}

bool SystemTray::event(QEvent *event)
{

}

SystemTray* SystemTray::system_tray_;

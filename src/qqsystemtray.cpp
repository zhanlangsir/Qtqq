#include "qqsystemtray.h"

#include <QEvent>
#include <QDebug>

QQSystemTray::QQSystemTray(QObject *parent) : QSystemTrayIcon(parent)
{

}

bool QQSystemTray::event(QEvent *event)
{

}

QQSystemTray* QQSystemTray::qq_system_tray_;

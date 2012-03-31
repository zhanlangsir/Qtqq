#pragma once

#include <QSystemTrayIcon>

class QQSystemTray : public QSystemTrayIcon
{
public:
    QQSystemTray(QObject *parent = 0);

protected:
    bool event(QEvent *event);
};

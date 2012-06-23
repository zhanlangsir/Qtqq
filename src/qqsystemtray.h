#pragma once

#include <QSystemTrayIcon>

class QQSystemTray : public QSystemTrayIcon
{
public:
    QQSystemTray(QObject *parent = 0);
    static QQSystemTray *instance()
    {
        if (!qq_system_tray_)
        {
            qq_system_tray_ = new QQSystemTray();
        }
        return qq_system_tray_;
    }

protected:
    bool event(QEvent *event);
private:
    static QQSystemTray *qq_system_tray_;
};

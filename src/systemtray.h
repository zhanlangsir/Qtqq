#ifndef QTQQ_SYSTEMTRAY_H
#define QTQQ_SYSTEMTRAY_H

#include <QSystemTrayIcon>

class SystemTray : public QSystemTrayIcon
{
public:
    static SystemTray *instance()
    {
        if (!system_tray_)
        {
            system_tray_ = new SystemTray();
        }
        return system_tray_;
    }

protected:
    bool event(QEvent *event);

private:
    SystemTray(QObject *parent = 0);

private:
    static SystemTray *system_tray_;
};

#endif  //QTQQ_SYSTEMTRAY_H

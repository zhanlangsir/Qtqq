#ifndef SYSTEMTRAY_H
#define SYSTEMTRAY_H

#include <QSystemTrayIcon>
#include <QMenu>

#include "core/qqmsg.h"

class QMenu;
class QAction;
class Roster;
class SysTrayActionHandle;

class SystemTrayIcon : public QSystemTrayIcon
{
	Q_OBJECT
signals:
	void sigActivateFriendRequestDlg(ShareQQMsgPtr msg);
	void sigActivateGroupRequestDlg(ShareQQMsgPtr msg);

public:
   	static SystemTrayIcon *instance()
    {
        if (!system_tray_)
        {
            system_tray_ = new SystemTrayIcon();
        }
        return system_tray_;
    }
    ~SystemTrayIcon(){}

	void activatedUnReadChat();

	void addMenuAction(QAction *act);
	void addNotifyAction(QAction *act);
	void removeAction(QAction *act);

	void addSeparator();

	bool hasNotify();

private:
	Roster *roster_;

	QMenu *menu_;

	QList<QAction *> actions_;

private:
	SystemTrayIcon();
	SystemTrayIcon(const SystemTrayIcon &);
	SystemTrayIcon &operator=(const SystemTrayIcon &);

	static SystemTrayIcon *system_tray_;
};

#endif //SYSTEMTRAY_H

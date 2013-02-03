#include "systemtray.h"

#include <QApplication>
#include <QMenu>
#include <QIcon>
#include <QAction>

#include "qqglobal.h"
#include "chatwidget/chatdlg_manager.h"
#include "soundplayer/soundplayer.h" 
#include "roster/roster.h"
#include "protocol/qq_protocol.h"

SystemTrayIcon* SystemTrayIcon::system_tray_ = NULL;

SystemTrayIcon::SystemTrayIcon() 
{
    qRegisterMetaType<SoundPlayer::SoundType>("SoundPlayer::SoundType");
	roster_ = Roster::instance();

	menu_ = new QMenu();

	this->setIcon(QIcon(QQGlobal::resourceDir() + "/webqq.ico"));
	this->setContextMenu(menu_);
}

void SystemTrayIcon::activatedUnReadChat()
{
	QAction *act = actions_.back();
	if ( act )
	{
		act->trigger();
	}
}

void SystemTrayIcon::addMenuAction(QAction *act)
{
	menu_->addAction(act);
}

void SystemTrayIcon::addNotifyAction(QAction *act)
{
	if ( actions_.isEmpty() )
		setIcon(QIcon(QQGlobal::resourceDir() + "/trayicon/newmsg.png"));

	menu_->addAction(act);
	actions_.push_back(act);
}

void SystemTrayIcon::removeAction(QAction *act)
{
	menu_->removeAction(act);
	actions_.removeOne(act);

	if ( actions_.isEmpty() )
		setIcon(QIcon(QQGlobal::resourceDir() + "/webqq.ico"));
}


void SystemTrayIcon::addSeparator()
{
	menu_->addSeparator();
}


bool SystemTrayIcon::hasNotify()
{
	return !actions_.isEmpty();
}

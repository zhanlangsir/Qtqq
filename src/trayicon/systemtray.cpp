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

    blink_timer_.setInterval(800);
    connect(&blink_timer_, SIGNAL(timeout()), this, SLOT(onBlinkTimeout()));
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

void SystemTrayIcon::addMenu(QAction *act)
{
	menu_->addAction(act);
}

void SystemTrayIcon::addNotifyAction(QAction *act)
{
    setIcon(act->icon());

	menu_->addAction(act);
	actions_.push_back(act);

    blink_timer_.start();
}

void SystemTrayIcon::removeAction(QAction *act)
{
	menu_->removeAction(act);
	actions_.removeOne(act);

	if ( actions_.isEmpty() )
    {
		setIcon(QIcon(QQGlobal::resourceDir() + "/webqq.ico"));
        blink_timer_.stop();
    }
    else
    {
        setIcon(actions_.last()->icon());
    }
}

void SystemTrayIcon::addSeparator()
{
	menu_->addSeparator();
}

bool SystemTrayIcon::hasNotify()
{
	return !actions_.isEmpty();
}

void SystemTrayIcon::onBlinkTimeout()
{
    static QIcon storage_icon = QIcon(); 

    if ( !icon().isNull() )
    {
        storage_icon = icon();
        setIcon(QIcon());
    }
    else
    {
        setIcon(storage_icon);
    }
}

#include "qtqq.h"

#include <QSettings>
#include <QMessageBox>

#include "chatwidget/chatdlg_manager.h"
#include "chatwidget/chatmsg_processor.h"
#include "loginwin.h"
#include "mainwindow.h"
#include "msgprocessor/msg_processor.h"
#include "notification_manager/notification_manager.h"
#include "protocol/qq_protocol.h"
#include "requestwidget/requestmsg_processor.h"
#include "roster/roster.h"
#include "skinengine/qqskinengine.h"
#include "skinengine/qqskinengine.h"
#include "strangermanager/stranger_manager.h"
#include "pluginmanager/plugin_manager.h"
#include "trayicon/systemtray.h"
#include "file_transfer/file_transfer_manager.h"
#include "hotkeymanager/hotkey_manager.h"
#include "setting/setting.h"

Qtqq *Qtqq::instance_ = NULL;

Qtqq::Qtqq() : login_dlg_(NULL),
    main_win_(NULL)
{
	SystemTrayIcon * tray  = SystemTrayIcon::instance();

	minimize_ = new QAction(QIcon(QQSkinEngine::instance()->skinRes("systray_minimize")), tr("Minimize"), NULL);
	connect(minimize_, SIGNAL(triggered()), this, SLOT(onMinimize()));

	restore_ = new QAction(QIcon(QQSkinEngine::instance()->skinRes("systray_restore")), tr("Restore"), NULL);
	connect(restore_, SIGNAL(triggered()), this, SLOT(onRestore()));

	//logout_ = new QAction(QIcon(QQSkinEngine::instance()->skinRes("systray_logout")), tr("Logout"), NULL);
	//connect(logout_, SIGNAL(triggered()), this, SLOT(onLogout()));

	quit_ = new QAction(QIcon(QQSkinEngine::instance()->skinRes("systray_quit")), tr("Quit"), NULL);
	connect(quit_, SIGNAL(triggered()), this, SLOT(onQuit()));

	tray->addMenu(minimize_);
	tray->addMenu(minimize_);
	tray->addMenu(restore_);
	//tray->addMenuAction(logout_);
	tray->addMenu(quit_);
	tray->addSeparator();
}

Qtqq::~Qtqq()
{
    if (login_dlg_)
    {
        login_dlg_->close();
        login_dlg_->deleteLater();
    }
    if (main_win_)
    {
    main_win_->close();
    main_win_->deleteLater();
    }
}

void Qtqq::start()
{
    login_dlg_ = new LoginWin();
    connect(login_dlg_, SIGNAL(sig_loginFinish()), this, SLOT(showMainPanel()));
}

void Qtqq::showMainPanel()
{
	//instantiate them
	Protocol::QQProtocol::instance();
	MsgProcessor::instance();
	ChatMsgProcessor::instance();
    NotificationManager::instance();
	RequestMsgProcessor::instance();
	StrangerManager::instance();
    Setting::instance();
	
	main_win_ = new MainWindow();

    HotkeyManager::instance();
    FileTransferManager::instance();

	main_win_->initialize();
    main_win_->show();
	main_win_->updateLoginUser();

    login_dlg_->deleteLater();
    login_dlg_ = NULL;

    PluginManager *plugin_mgr = PluginManager::instance();
    plugin_mgr->loadSettings();
    plugin_mgr->loadPlugins();

	SystemTrayIcon *trayicon = SystemTrayIcon::instance();
	trayicon->show();
}

void Qtqq::onMinimize()
{
    main_win_->hide();
}

void Qtqq::onRestore()
{
    main_win_->showNormal();
}

void Qtqq::onLogout()
{
	Protocol::QQProtocol::instance()->stop();
	MsgProcessor::instance()->stop();
	ChatMsgProcessor::instance()->stop();
	RequestMsgProcessor::instance()->stop();

	delete StrangerManager::instance();
	delete ChatDlgManager::instance();
	delete Roster::instance();
    delete main_win_;
    HotkeyManager::instance()->reset();
    Setting::instance()->reset();

	SystemTrayIcon *trayicon = SystemTrayIcon::instance();
	trayicon->hide();

    start();
}

void Qtqq::onQuit()
{
	delete Protocol::QQProtocol::instance();
	delete MsgProcessor::instance();
	delete ChatMsgProcessor::instance();
	delete RequestMsgProcessor::instance();
	delete StrangerManager::instance();
	delete ChatDlgManager::instance();
	delete Roster::instance();
    delete PluginManager::instance();
    delete HotkeyManager::instance();
    delete FileTransferManager::instance();
    delete Setting::instance();

	if ( main_win_ )
		delete main_win_;
	main_win_ = NULL;

	delete minimize_;
	minimize_ = NULL;

	delete restore_;
	restore_ = NULL;

	//delete logout_;
	//logout_ = NULL;

	delete quit_;
	quit_ = NULL;

	qApp->quit();
}

void Qtqq::aboutQtqq()
{
    QMessageBox::information(NULL, tr("About Qtqq"), 
            tr("Version: %1\n\n"
                "用c++和Qt写的基于webqq3.0协议的Linux qq客户端!\n\n"
                "主页: http://www.aitilife.com/qtqq\n"
                "代码托管在: https://github.com/zhanlangsir/Qtqq").arg(QQGlobal::version()));
}

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QDesktopWidget>
#include <QEvent>
#include <QFile>
#include <QHttp>
#include <QSettings>
#include <QDebug>

#include "qxtglobalshortcut.h"
#include "json/json.h"

#include "chatwidget/chatdlg_manager.h"
#include "chatwidget/chatmsg_processor.h"
#include "chatwidget/friendchatdlg.h"
#include "chatwidget/groupchatdlg.h"
#include "core/captchainfo.h"
#include "core/curr_login_account.h"
#include "core/qqavatarrequester.h"
#include "core/qqlogincore.h"
#include "core/qqutility.h"
#include "core/sockethelper.h"
#include "core/types.h"
#include "event_handle/event_handle.h"
#include "friendsearcher.h"
#include "msgprocessor/msg_processor.h"
#include "protocol/event_center.h"
#include "protocol/qq_protocol.h"
#include "qqglobal.h"
#include "qqiteminfohelper.h"
#include "requestwidget/requestmsg_processor.h"
#include "rostermodel/contact_proxy_model.h"
#include "rostermodel/recent_model.h"
#include "rostermodel/roster_model.h"
#include "roster/roster.h"
#include "skinengine/qqskinengine.h"
#include "trayicon/systemtray.h"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow),
    main_http_(new QHttp),
	contact_model_(NULL),
	group_model_(NULL),
	recent_model_(NULL),
	open_chat_dlg_sc_(NULL)
{
    ui->setupUi(this);

    qRegisterMetaType<ContactClientType>("ContactClientType");

    setObjectName("mainWindow");
    setWindowIcon(QIcon(QQGlobal::instance()->appIconPath()));
    setWindowTitle(CurrLoginAccount::name());

	ui->tv_friendlist->setSortingEnabled(true);

    setupLoginStatus();

    move((QApplication::desktop()->width() - this->width()) /2, (QApplication::desktop()->height() - this->height()) /2);

    
    connect(ui->cb_status, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMyStatus(int)));
    connect(ui->pb_mainmenu, SIGNAL(clicked()), this, SLOT(openMainMenu()));
    connect(Protocol::EventCenter::instance(), SIGNAL(eventTrigger(Protocol::Event *)), EventHandle::instance(), SLOT(onEventTrigger(Protocol::Event *)));

    convertor_.addUinNameMap(CurrLoginAccount::id(), tr("you"));

    if (QFile::exists(QQGlobal::configDir() + "/qqgroupdb"))
    {
        QFile::remove(QQGlobal::configDir() + "/qqgroupdb");
    }

    QSettings setting(QQGlobal::configDir() + "/options.ini", QSettings::IniFormat);
    main_menu_ = new QMenu(this);
    act_mute_ = new QAction(tr("Mute"), main_menu_);
    act_mute_->setCheckable(true);
    act_mute_->setChecked(setting.value("mute").toBool());

    connect(act_mute_, SIGNAL(toggled(bool)), this, SLOT(setMute(bool)));

    main_menu_->addAction(act_mute_);

    if (!open_chat_dlg_sc_)
    {
        open_chat_dlg_sc_ = new QxtGlobalShortcut(QKeySequence("Ctrl+Alt+Z"), this);
        connect(open_chat_dlg_sc_, SIGNAL(activated()), this, SLOT(openFirstChatDlg()));
    }
}

MainWindow::~MainWindow()
{
	if ( ui )
		delete ui;
	ui = NULL;

	clean();

	if ( contact_model_ )
		delete contact_model_;
	contact_model_ = NULL;

	if ( group_model_ )
		delete group_model_;
	group_model_ = NULL;

	if ( recent_model_ )
		delete recent_model_;
	recent_model_ = NULL;

	if ( main_http_ )
	{
		main_http_->close();
		delete main_http_;
		main_http_ = NULL;
	}
}

void MainWindow::clean()
{
	if ( contact_model_ )
		contact_model_->clean();

	if ( group_model_ )
		group_model_->clean();

	if ( recent_model_ )
		recent_model_->clean();

    main_http_->close();
}

void MainWindow::setMute(bool mute)
{
    QSettings setting(QQGlobal::configDir() + "/options.ini", QSettings::IniFormat);
    setting.setValue("mute", mute);
}

void MainWindow::openMainMenu()
{
    main_menu_->popup(QCursor::pos());
}

void MainWindow::changeMyStatus(int idx)
{
    QString change_status_url = "/channel/change_status2?newstatus=" + getStatusByIndex(idx) + 
        "&clientid=5412354841&psessionid=" + CaptchaInfo::instance()->psessionid() + 
        "&t=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());

    QHttpRequestHeader header("GET", change_status_url);
    header.addValue("Host", "d.web2.qq.com");
    setDefaultHeaderValue(header);
    header.addValue("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002&callback=1");
    header.addValue("Cookie", CaptchaInfo::instance()->cookie());

    main_http_->setHost("d.web2.qq.com");
    main_http_->request(header);

	CurrLoginAccount::setStatus(ui->cb_status->itemData(idx).value<ContactStatus>());

	updateLoginUser();
}

void MainWindow::closeEvent(QCloseEvent *)
{
    if (QFile::exists(QQGlobal::configDir() + "/qqgroupdb"))
    {
        QFile::remove(QQGlobal::configDir() + "/qqgroupdb");
    }
}

void MainWindow::getFriendList()
{
    QString get_friendlist_url = "/api/get_user_friends2";
    QString msg_content = "r={\"h\":\"hello\",\"vfwebqq\":\"" + CaptchaInfo::instance()->vfwebqq() + "\"}";

    QHttpRequestHeader header("POST", get_friendlist_url);
    header.addValue("Host", "s.web2.qq.com");
    setDefaultHeaderValue(header);
    header.addValue("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001&callback=1&id=1");
    header.addValue("Cookie", CaptchaInfo::instance()->cookie());
    header.setContentType("application/x-www-form-urlencoded");
    header.setContentLength(msg_content.length());

    main_http_->setHost("s.web2.qq.com");
    connect(main_http_, SIGNAL(done(bool)), this, SLOT(getFriendListDone(bool)));
    main_http_->request(header, msg_content.toAscii());
}

void MainWindow::getFriendListDone(bool err)
{
    Q_UNUSED(err)
    disconnect(main_http_, SIGNAL(done(bool)), this, SLOT(getFriendListDone(bool)));
    QByteArray contact_info = main_http_->readAll();
	qDebug() << "Contact list:\n" << contact_info << endl;

	contact_model_ = new RosterModel(this);
	ContactProxyModel *contact_proxy_model = new ContactProxyModel(contact_model_);
	contact_proxy_model->setSourceModel(contact_model_);
	contact_model_->setProxyModel(contact_proxy_model);

    connect(ui->tv_friendlist, SIGNAL(doubleClicked(const QModelIndex &)), contact_model_, SLOT(onDoubleClicked(const QModelIndex &)));
	Roster *roster = Roster::instance();
	connect(roster, SIGNAL(sigNewCategory(const Category *)), contact_model_, SLOT(addCategoryItem(const Category *)));
	connect(roster, SIGNAL(sigContactDataChanged(QString, QVariant, TalkableDataRole)), contact_model_, SLOT(talkableDataChanged(QString, QVariant, TalkableDataRole)));
	connect(roster, SIGNAL(sigCategoryDataChanged(int, QVariant, TalkableDataRole)), contact_model_, SLOT(categoryDataChanged(int, QVariant, TalkableDataRole)));

	connect(roster, SIGNAL(sigNewContact(const Contact *)), contact_model_, SLOT(addContactItem(const Contact *)));

	roster->parseContactList(contact_info);

    FriendSearcher *friend_searcher = new FriendSearcher();
	//may initialize after the friend list had set up
	friend_searcher->initialize();
	contact_proxy_model->setFilter(friend_searcher);
	connect(ui->le_search, SIGNAL(textChanged(const QString &)), contact_proxy_model, SLOT(onSearch(const QString &)));

    ui->tv_friendlist->setModel(contact_proxy_model);

    getGroupList();
}

void MainWindow::getSingleLongNick()
{
    QByteArray result = QQItemInfoHelper::getSingleLongNick(CurrLoginAccount::id());
    result = result.mid(result.indexOf("\r\n\r\n")+4);
    Json::Reader reader;
    Json::Value root;

    if (reader.parse(QString(result).toStdString(), root, false))
    {
         ui->le_mood->setText(QString::fromStdString(root["result"][0]["lnick"].asString()));
    }
}

void MainWindow::getGroupList()
{
    QString get_grouplist_url = "/api/get_group_name_list_mask2";
    QString msg_content = "r={\"h\":\"hello\",\"vfwebqq\":\"" + CaptchaInfo::instance()->vfwebqq() + "\"}";
    QHttpRequestHeader header("POST", get_grouplist_url);
    header.addValue("Host", "s.web2.qq.com");
    setDefaultHeaderValue(header);
    header.addValue("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001&callback=1&id=1");
    header.addValue("Cookie", CaptchaInfo::instance()->cookie());
    header.setContentType("application/x-www-form-urlencoded");
    header.setContentLength(msg_content.length());

    main_http_->setHost("s.web2.qq.com");
    connect(main_http_, SIGNAL(done(bool)), this, SLOT(getGroupListDone(bool)));
    main_http_->request(header, msg_content.toAscii());
}

void MainWindow::getGroupListDone(bool err)
{
    Q_UNUSED(err)
    disconnect(main_http_, SIGNAL(done(bool)), this, SLOT(getGroupListDone(bool)));
    QByteArray groups_info = main_http_->readAll();
	qDebug() << "Group List: \n" << groups_info << endl;

	group_model_ = new RosterModel(this);
    connect(ui->lv_grouplist, SIGNAL(doubleClicked(const QModelIndex &)), group_model_, SLOT(onDoubleClicked(const QModelIndex &)));
	Roster *roster = Roster::instance();
	connect(roster, SIGNAL(sigNewGroup(const Group *)), group_model_, SLOT(addGroupItem(const Group *)));
	connect(roster, SIGNAL(sigGroupDataChanged(QString, QVariant, TalkableDataRole)), group_model_, SLOT(talkableDataChanged(QString, QVariant, TalkableDataRole)));

	roster->parseGroupList(groups_info);

	ui->lv_grouplist->setModel(group_model_);

	getRecentList();
}

void MainWindow::getOnlineBuddy()
{
	QString get_online_buddy = "/channel/get_online_buddies2?clientid=5412354841&psessionid=" + CaptchaInfo::instance()->psessionid() + "&t=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
	QHttpRequestHeader header("GET", get_online_buddy);
	header.addValue("Host", "d.web2.qq.com");
	setDefaultHeaderValue(header);
	header.addValue("Referer", "http://d.web2.qq.com/proxy.html?v=2011033100");
	header.addValue("Cookie", CaptchaInfo::instance()->cookie());

	main_http_->setHost("d.web2.qq.com");
	connect(main_http_, SIGNAL(done(bool)), this, SLOT(getOnlineBuddyDone(bool)));
	main_http_->request(header);
}

void MainWindow::getOnlineBuddyDone(bool err)
{
	Q_UNUSED(err)
	disconnect(main_http_, SIGNAL(done(bool)), this, SLOT(getOnlineBuddyDone(bool)));
	QByteArray online_buddies = main_http_->readAll();
	Roster::instance()->parseContactStatus(online_buddies);

	Protocol::QQProtocol *protocol = Protocol::QQProtocol::instance();
	MsgProcessor *msg_processor = MsgProcessor::instance();
	connect(protocol, SIGNAL(newQQMsg(QByteArray)), msg_processor, SLOT(pushRawMsg(QByteArray)));
    connect(msg_processor, SIGNAL(contactStatusChanged(QString, ContactStatus, ContactClientType)), Roster::instance(), SLOT(slotContactStatusChanged(QString, ContactStatus, ContactClientType)));

	protocol->run();
	msg_processor->start();
}

void MainWindow::getPersonalFace()
{
	QString avatar_path = QQAvatarRequester::requestOne(QQAvatarRequester::getTypeNumber(QQItem::kFriend), CurrLoginAccount::id(), QQGlobal::tempDir());
	CurrLoginAccount::setAvatarPath(avatar_path);

	QFile file(avatar_path);
	file.open(QIODevice::ReadOnly);
	QPixmap pix;
	pix.loadFromData(file.readAll());
	file.close();
	ui->avatar->setIconSize(QSize(pix.size().width() + 4, pix.size().height()+4));
	ui->avatar->setIcon(QIcon(pix));
}

void MainWindow::getPersonalInfo()
{
	QByteArray result = QQItemInfoHelper::getFriendInfo2(CurrLoginAccount::id());

    qDebug() << "Persion Infomation: \n"
             << result << endl;

	result = result.mid(result.indexOf("\r\n\r\n")+4);
	Json::Reader reader;
	Json::Value root;

	if (reader.parse(QString(result).toStdString(), root, false))
	{
		CurrLoginAccount::setName(QString::fromStdString(root["result"]["nick"].asString()));
		ui->lbl_name->setText(CurrLoginAccount::name());

        int vip_level = root["result"]["vip_info"].asInt();
        if ( vip_level > 0 )
        {
            QString vip_label = "Vip " + QString::number(vip_level);
            ui->vip_label->setText(vip_label);
        }
        else
            ui->vip_label->setText("");

		convertor_.addUinNameMap(CurrLoginAccount::id(), CurrLoginAccount::name());
	} 
}

void MainWindow::getRecentList()
{
	QString recent_list_url ="/channel/get_recent_list2";
	QString msg_content = "r={\"vfwebqq\":\"" + CaptchaInfo::instance()->vfwebqq() +
		"\",\"clientid\":\"5412354841\",\"psessionid\":\"" + CaptchaInfo::instance()->psessionid() +
		"\"}&cliendid=5412354841&psessionid=" + CaptchaInfo::instance()->psessionid();

	QHttpRequestHeader header("POST", recent_list_url);
	header.addValue("Host", "d.web2.qq.com");
	setDefaultHeaderValue(header);
	header.addValue("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002&callback=1");
	header.addValue("Cookie", CaptchaInfo::instance()->cookie());
	header.setContentType("application/x-www-form-urlencoded");
	header.setContentLength(msg_content.length());

	main_http_->setHost("d.web2.qq.com");
	connect(main_http_, SIGNAL(done(bool)), this, SLOT(getRecentListDone(bool)));
	main_http_->request(header, msg_content.toAscii());
}

void MainWindow::getRecentListDone(bool err)
{
	Q_UNUSED(err)
	disconnect(main_http_, SIGNAL(done(bool)), this, SLOT(getRecentListDone(bool)));
	QByteArray recent_list = main_http_->readAll();
	
	recent_model_ = new RecentModel(this);
    connect(ui->lv_recentlist, SIGNAL(doubleClicked(const QModelIndex &)), recent_model_, SLOT(onDoubleClicked(const QModelIndex &)));

	connect(MsgProcessor::instance(), SIGNAL(newChatMsg(ShareQQMsgPtr)), recent_model_, SLOT(slotNewChatMsg(ShareQQMsgPtr)));

	recent_model_->parseRecentContact(recent_list);
	ui->lv_recentlist->setModel(recent_model_);

	getOnlineBuddy();
}

void MainWindow::initialize()
{
	getPersonalInfo();
	getPersonalFace();
	getSingleLongNick();

	getFriendList();

	ChatDlgManager *chat_manager = ChatDlgManager::instance();
	chat_manager->setMainWin(this);
}

void MainWindow::openFirstChatDlg()
{
	SystemTrayIcon *tray = SystemTrayIcon::instance();

	if ( tray->hasNotify() )
	{
		tray->activatedUnReadChat();
	}
	else
	{
		if ( !isVisible() )
			showNormal();

		activateWindow();
	}
}

QString MainWindow::getStatusByIndex(int idx) const
{
	switch (ui->cb_status->itemData(idx).value<ContactStatus>())
	{
		case CS_Online:
			return "online";
		case CS_CallMe:
			return "callme";
		case CS_Away:
			return "away";
		case CS_Busy:
			return "busy";
		case CS_Silent:
			return "silent";
		case CS_Hidden:
			return "hidden";
		case CS_Offline:
			return "offline";
		default:
			break;
	}
	return "offline";
}

void MainWindow::setupLoginStatus()
{
	ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->skinRes("status_online")), tr("Online"), QVariant::fromValue<ContactStatus>(CS_Online));
	ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->skinRes("status_qme")), tr("CallMe"), QVariant::fromValue<ContactStatus>(CS_CallMe));
	ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->skinRes("status_away")), tr("Away"), QVariant::fromValue<ContactStatus>(CS_Away));
	ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->skinRes("status_busy")), tr("Busy"), QVariant::fromValue<ContactStatus>(CS_Busy));
	ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->skinRes("status_mute")), tr("Silent"), QVariant::fromValue<ContactStatus>(CS_Silent));
	ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->skinRes("status_hidden")), tr("Hidden"), QVariant::fromValue<ContactStatus>(CS_Hidden));
	ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->skinRes("status_offline")), tr("Offline"), QVariant::fromValue<ContactStatus>(CS_Offline));

	int status_idx = getStatusIndex(CurrLoginAccount::status());

	ui->cb_status->setCurrentIndex(status_idx);
}

int MainWindow::getStatusIndex(ContactStatus status)
{
	for (int i = 0; i < ui->cb_status->count(); ++i)
	{
		if (ui->cb_status->itemData(i).value<ContactStatus>() == status)
			return i;
	}
	return -1;
}

void MainWindow::updateLoginUser() const
{
	SystemTrayIcon *tray = SystemTrayIcon::instance();

	QString tooltip("qtqq - ");

	tooltip += CurrLoginAccount::id() + "\n";
	tooltip += CurrLoginAccount::name() + " (";
	tooltip += QQUtility::StatusToString(CurrLoginAccount::status()) + ")";
}

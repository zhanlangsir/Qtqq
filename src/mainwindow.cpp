#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopWidget>
#include <QModelIndex>
#include <QMessageBox>
#include <QEvent>
#include <QCursor>
#include <QFile>
#include <QSettings>
#include <QJSEngine>
#include <QDebug>

#include "qxtglobalshortcut.h"
#include "json/json.h"

#include "chatwidget/chatdlg_manager.h"
#include "chatwidget/chatmsg_processor.h"
#include "chatwidget/friendchatdlg.h"
#include "core/qqavatarrequester.h"
#include "chatwidget/groupchatdlg.h"
#include "core/captchainfo.h"
#include "core/curr_login_account.h"
#include "core/qqutility.h"
#include "core/sockethelper.h"
#include "event_handle/event_handle.h"
#include "msgprocessor/msg_processor.h"
#include "pluginmanager/plugin_managedlg.h"
#include "protocol/event_center.h"
#include "protocol/qq_protocol.h"
#include "requestwidget/requestmsg_processor.h"
#include "rostermodel/contact_proxy_model.h"
#include "rostermodel/recent_model.h"
#include "rostermodel/roster_model.h"
#include "rosterview/rosterview.h"
#include "roster/roster.h"
#include "skinengine/qqskinengine.h"
#include "trayicon/systemtray.h"
#include "rostermodel/contact_searcher.h"
#include "rostermodel/roster_index.h"
#include "utils/menu.h"
#include "hotkeymanager/hotkey_manager.h"
#include "snapshot/ksnapshot.h"
#include "shortcutmanager/shortcut_manager.h"
#include "qqglobal.h"
#include "qqiteminfohelper.h"
#include "setting/setting.h"
#include "qtqq.h"

#define OPEN_CHATDLG_SC "open-chatdlg"
#define SC_SNAPSHOT "Global.Snapshot"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow),
    friend_view_(new RosterView(this)),
    group_view_(new RosterView(this)),
    recent_view_(new RosterView(this)),
	contact_model_(NULL),
	group_model_(NULL),
	recent_model_(NULL)
{
    ui->setupUi(this);

    qRegisterMetaType<ContactClientType>("ContactClientType");
    setObjectName("mainWindow");

    initUi();
    connect(ui->cb_status, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMyStatus(int)));
    connect(Protocol::EventCenter::instance(), SIGNAL(eventTrigger(Protocol::Event *)), EventHandle::instance(), SLOT(onEventTrigger(Protocol::Event *)));

    if (QFile::exists(QQGlobal::configDir() + "/qqgroupdb"))
    {
        QFile::remove(QQGlobal::configDir() + "/qqgroupdb");
    }

    main_menu_ = new Menu(this);
    act_mute_ = new QAction(tr("Mute"), main_menu_);
    act_mute_->setCheckable(true);
    act_mute_->setChecked(Setting::instance()->value("mute") == "True" ? true : false);
    connect(act_mute_, SIGNAL(toggled(bool)), this, SLOT(setMute(bool)));

    QAction *quit = new QAction(tr("Quit"), main_menu_);
    connect(quit, SIGNAL(triggered()), Qtqq::instance(), SLOT(onQuit()));
    QAction *about_qtqq = new QAction(tr("About Qtqq"), main_menu_);
    connect(about_qtqq, SIGNAL(triggered()), Qtqq::instance(), SLOT(aboutQtqq()));
    QAction *about_qt = new QAction(tr("About Qt"), main_menu_);
    connect(about_qt, SIGNAL(triggered()), this,  SLOT(aboutQt()));

    connect(ui->mainmenu_btn, SIGNAL(clicked()), this, SLOT(onMainMenuclicked()));

    main_menu_->addAction(act_mute_);
    main_menu_->addPluginSperator();
    main_menu_->addAction(about_qtqq);
    main_menu_->addAction(about_qt);
    main_menu_->addSeparator();
    main_menu_->addAction(quit);
    
    SystemTrayIcon *tray = SystemTrayIcon::instance();
    connect(tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onTrayIconClicked(QSystemTrayIcon::ActivationReason)));
}

MainWindow::~MainWindow()
{
	if ( ui )
		delete ui;
	ui = NULL;

	clean();

#define CLEAN(x) { \
    if ( x ) \
    { \
        delete x; \
        x = NULL; \
    } \
}
    CLEAN(contact_model_)
    CLEAN(group_model_)
    CLEAN(recent_model_)
#undef CLEAN
}

void MainWindow::initUi()
{
    setWindowIcon(QIcon(QQGlobal::instance()->appIconPath()));
    setWindowTitle(CurrLoginAccount::name());

    friend_view_->setHeaderHidden(true);
    group_view_->setHeaderHidden(true);
    recent_view_->setHeaderHidden(true);
    ui->tab_main->insertTab(0, friend_view_, tr("friend"));
    ui->tab_main->insertTab(1, group_view_, tr("group"));
    ui->tab_main->insertTab(2, recent_view_, tr("recent"));

    friend_view_->setSortingEnabled(true);

    move((QApplication::desktop()->width() - this->width()) /2, (QApplication::desktop()->height() - this->height()) /2);
    setupLoginStatus();
}

void MainWindow::snapshot()
{
    //it will be release in KSnapshot class
    KSnapshot *snap = new KSnapshot();
    Q_UNUSED(snap)
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(NULL);
}

void MainWindow::onTrayIconClicked(QSystemTrayIcon::ActivationReason reason)
{
    if ( reason != QSystemTrayIcon::Trigger )
        return;

    SystemTrayIcon *tray = SystemTrayIcon::instance();
    if ( tray->hasNotify() )
    {
        tray->activatedUnReadChat();
        return;
    }
    
    if (isMinimized() || !isVisible())
        showNormal();
    else
        hide();
}

void MainWindow::clean()
{
	if ( contact_model_ )
		contact_model_->clean();

	if ( group_model_ )
		group_model_->clean();

	if ( recent_model_ )
		recent_model_->clean();

    ShortcutManager::instance()->removeShortcut(OPEN_CHATDLG_SC);
    ShortcutManager::instance()->removeShortcut(SC_SNAPSHOT);
}

void MainWindow::setMute(bool mute)
{
    Setting::instance()->setValue("mute", mute ? "True" : "False");
}

void MainWindow::openMainMenu()
{
    main_menu_->popup(QCursor::pos());
}

void MainWindow::changeMyStatus(int idx)
{
    ContactStatus new_status = ui->cb_status->itemData(idx).value<ContactStatus>();
    Protocol::QQProtocol::instance()->changeStatus(new_status);
	CurrLoginAccount::setStatus(new_status);

	updateLoginUser();
}

void MainWindow::closeEvent(QCloseEvent *)
{
    if (QFile::exists(QQGlobal::configDir() + "/qqgroupdb"))
    {
        QFile::remove(QQGlobal::configDir() + "/qqgroupdb");
    }
}

QString MainWindow::hashO(const QString &uin, const QString &ptwebqq)
{
    QString a;
    a.append(ptwebqq);
    a.append("password error");

    QString s;
    while ( true )
    {
        if ( s.length() < a.length() )
        {
            s.append(uin);
            if ( s.length() == a.length() )
                break;
        }
        else
        {
            s.truncate(a.length());
            break;
        }
    }

    QString j;
    for ( int i = 0; i < s.length(); ++i )
    {
        j.append(s[i].toLatin1() ^ a[i].toLatin1());
    }

    QString key = "0123456789ABCDEF";

    s[0] = 0;
    for ( int i = 0; i < a.length(); ++i )
    {
        s[2*i] = key[j[i].toLatin1() >> 4 & 15];
        s[2*i+1] = key[j[i].toLatin1() & 15];
    }
    return s;
}

void MainWindow::getFriendList()
{
    QNetworkReply *reply = Protocol::QQProtocol::instance()->getFriendList();
    connect(reply, SIGNAL(finished()), this, SLOT(getFriendListDone()));
}

void MainWindow::getFriendListDone()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    QByteArray contact_info = reply->readAll();
    reply->deleteLater();
    qDebug() << "Contact list:\n" << contact_info << endl;

    contact_model_ = new RosterModel(this);
    contact_proxy_model_ = new ContactProxyModel(contact_model_);
    contact_proxy_model_->setSourceModel(contact_model_);
    contact_model_->setProxyModel(contact_proxy_model_);

    connect(friend_view_, SIGNAL(doubleClicked(const QModelIndex &)), contact_model_, SLOT(onDoubleClicked(const QModelIndex &)));
    Roster *roster = Roster::instance();
    connect(roster, SIGNAL(sigNewCategory(Category *)), contact_model_, SLOT(addCategoryItem(Category *)));
    connect(roster, SIGNAL(beginClean()), contact_model_, SLOT(clean()));
    connect(roster, SIGNAL(sigContactDataChanged(QString, QVariant, TalkableDataRole)), contact_model_, SLOT(talkableDataChanged(QString, QVariant, TalkableDataRole)));
    connect(roster, SIGNAL(sigCategoryDataChanged(int, QVariant, TalkableDataRole)), contact_model_, SLOT(categoryDataChanged(int, QVariant, TalkableDataRole)));

    connect(roster, SIGNAL(sigNewContact(Contact *)), contact_model_, SLOT(addContactItem(Contact *)));

    roster->parseContactList(contact_info);

    searcher_ = new ContactSearcher(this);
    searcher_->initialize(Roster::instance()->contacts());
    connect(ui->le_search, SIGNAL(textChanged(const QString &)), this, SLOT(onSearch(const QString &)));

    friend_view_->setProxyModel(contact_proxy_model_);
    friend_view_->setRosterModel(contact_model_);

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
    QNetworkReply *reply = Protocol::QQProtocol::instance()->getGroupList();
    connect(reply, SIGNAL(finished()), this, SLOT(getGroupListDone()));
}

void MainWindow::getGroupListDone()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    QByteArray groups_info = reply->readAll();
    reply->deleteLater();
    qDebug() << "Group List: \n" << groups_info << endl;

    group_model_ = new RosterModel(this);
    connect(group_view_, SIGNAL(doubleClicked(const QModelIndex &)), group_model_, SLOT(onDoubleClicked(const QModelIndex &)));
    Roster *roster = Roster::instance();
    connect(roster, SIGNAL(sigNewGroup(Group *)), group_model_, SLOT(addGroupItem(Group *)));
    connect(roster, SIGNAL(beginClean()), group_model_, SLOT(clean()));
    connect(roster, SIGNAL(sigGroupDataChanged(QString, QVariant, TalkableDataRole)), group_model_, SLOT(talkableDataChanged(QString, QVariant, TalkableDataRole)));

    roster->parseGroupList(groups_info);

    group_view_->setRosterModel(group_model_);

    getRecentList();
}

void MainWindow::getOnlineBuddy()
{
    QNetworkReply *reply = Protocol::QQProtocol::instance()->getOnlineBuddy();
    connect(reply, SIGNAL(finished()), this, SLOT(getOnlineBuddyDone()));
}

void MainWindow::getOnlineBuddyDone()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    QByteArray online_buddies = reply->readAll();
    reply->deleteLater();
    Roster::instance()->parseContactStatus(online_buddies);

    Protocol::QQProtocol *protocol = Protocol::QQProtocol::instance();
    MsgProcessor *msg_processor = MsgProcessor::instance();
    connect(protocol, SIGNAL(newQQMsg(QByteArray)), msg_processor, SLOT(pushRawMsg(QByteArray)));
    connect(msg_processor, SIGNAL(contactStatusChanged(QString, ContactStatus, ContactClientType)), Roster::instance(), SLOT(slotContactStatusChanged(QString, ContactStatus, ContactClientType)));

    protocol->run();
    msg_processor->start();
}

void MainWindow::stop()
{
    Protocol::QQProtocol *protocol = Protocol::QQProtocol::instance();
    MsgProcessor *msg_processor = MsgProcessor::instance();
    disconnect(protocol, SIGNAL(newQQMsg(QByteArray)), msg_processor, SLOT(pushRawMsg(QByteArray)));
    disconnect(msg_processor, SIGNAL(contactStatusChanged(QString, ContactStatus, ContactClientType)), Roster::instance(), SLOT(slotContactStatusChanged(QString, ContactStatus, ContactClientType)));
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
    } 
}

void MainWindow::getRecentList()
{
    QNetworkReply *reply = Protocol::QQProtocol::instance()->getRecentList();
    connect(reply, SIGNAL(finished()), this, SLOT(getRecentListDone()));
}

void MainWindow::getRecentListDone()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    QByteArray recent_list = reply->readAll();
    reply->deleteLater();
	
    recent_model_ = new RecentModel(this);
    connect(recent_view_, SIGNAL(doubleClicked(const QModelIndex &)), recent_model_, SLOT(onDoubleClicked(const QModelIndex &)));

    connect(MsgProcessor::instance(), SIGNAL(newChatMsg(ShareQQMsgPtr)), recent_model_, SLOT(slotNewChatMsg(ShareQQMsgPtr)));
    connect(Roster::instance(), SIGNAL(beginClean()), recent_model_, SLOT(clean()));

    recent_model_->parseRecentContact(recent_list);

    recent_view_->setRosterModel(recent_model_);

    getOnlineBuddy();
}

void MainWindow::initialize()
{
    initShortcut();

    getPersonalInfo();
    getPersonalFace();
    getSingleLongNick();

    getFriendList();

    ChatDlgManager *chat_manager = ChatDlgManager::instance();
    ChatDlgManager::instance()->initConnections();
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
        {
            showNormal();
            activateWindow();
            raise();
        }
        else
        {
            hide();
        }
    }
}

void MainWindow::setupLoginStatus()
{
    ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->skinRes("status_online")), tr("Online"), QVariant::fromValue<ContactStatus>(CS_Online));
    ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->skinRes("status_qme")), tr("CallMe"), QVariant::fromValue<ContactStatus>(CS_CallMe));
    ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->skinRes("status_away")), tr("Away"), QVariant::fromValue<ContactStatus>(CS_Away));
    ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->skinRes("status_busy")), tr("Busy"), QVariant::fromValue<ContactStatus>(CS_Busy));
    ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->skinRes("status_silent")), tr("Silent"), QVariant::fromValue<ContactStatus>(CS_Silent));
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
    QString tooltip("qtqq - ");

    tooltip += CurrLoginAccount::id() + "\n";
    tooltip += CurrLoginAccount::name() + " (";
    tooltip += QQUtility::StatusToString(CurrLoginAccount::status()) + ")";
}

void MainWindow::onSearch(const QString &str)
{
    if ( str.isEmpty() )
        contact_proxy_model_->endFilter();
    else
    {
        QVector<QString> result;
        searcher_->search(str, result);

        //contact's category also should be shown
        foreach ( const QString &id, result )
        {
            Contact *contact = Roster::instance()->contact(id);
            assert(contact->category());
            QString cat_index = QString::number(contact->category()->index());
            if ( !result.contains(cat_index) )
                result.append(cat_index);
        }

        contact_proxy_model_->setFilter(result);
    }
}

void MainWindow::onMainMenuclicked()
{
    QPoint pos;
    pos.setX(0);
    pos.setY(-main_menu_->sizeHint().height());
    main_menu_->exec(ui->mainmenu_btn->mapToGlobal(pos));
}

void MainWindow::initShortcut()
{
    const Shortcut *open_chat_dlg_sc = ShortcutManager::instance()->createGlobalShortcut(OPEN_CHATDLG_SC, tr("Active Chat Window"), QKeySequence(ShortcutManager::instance()->keyFromShortcutId(OPEN_CHATDLG_SC, SCG_GLOBAL "Ctrl+Alt+Z")), this);
    connect(open_chat_dlg_sc, SIGNAL(activated()), this, SLOT(openFirstChatDlg()));

    const Shortcut *snapshot = ShortcutManager::instance()->createGlobalShortcut(SC_SNAPSHOT, tr("Snapshot"), QKeySequence(ShortcutManager::instance()->keyFromShortcutId(SC_SNAPSHOT, SCG_GLOBAL, "Ctrl+Alt+S")), this);
    connect(snapshot, SIGNAL(activated()), this, SLOT(snapshot()));
}

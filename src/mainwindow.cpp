#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopWidget>
#include <QHttp>
#include <QDebug>
#include <QTreeWidgetItem>
#include <QSemaphore>
#include <QDateTime>
#include <assert.h>
#include <QFile>
#include <QSettings>
#include <QEvent>

#include "qxtglobalshortcut.h"
#include <json/json.h>

#include "core/qqavatarrequester.h"
#include "core/types.h"
#include "core/qqutility.h"
#include "core/msgcenter.h"
#include "core/pollthread.h"
#include "core/parsethread.h"
#include "core/qqlogincore.h"
#include "core/captchainfo.h"
#include "frienditemmodel.h"
#include "groupitemmodel.h"
#include "grouprequestdlg.h"
#include "qqiteminfohelper.h"
#include "friendrequestdlg.h"
#include "friendchatdlg.h"
#include "msgtip.h"
#include "groupchatdlg.h"
#include "core/qqskinengine.h"
#include "core/sockethelper.h"
#include "traymenu.h"
#include "traymenuitem.h"
#include "core/msgencoder.h"
#include "core/friendmsgencoder.h"
#include "core/groupmsgencoder.h"
#include "qqglobal.h"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow),
    main_http_(new QHttp),
    poll_thread_(NULL),
    parse_thread_(NULL),
    message_queue_(new QQueue<QByteArray>()),
    msg_tip_(new MsgTip()),
    msg_center_(new MsgCenter(msg_tip_)),
    open_chat_dlg_sc_(NULL)
{
    ui->setupUi(this);

    qRegisterMetaType<ClientType>("ClientType");

    setObjectName("mainWindow");
    setWindowIcon(QIcon(QQSkinEngine::instance()->getSkinRes("app_icon")));
    setWindowTitle(QQSettings::instance()->currLoginInfo().name);

    createTray();

    setupLoginStatus();

    move((QApplication::desktop()->width() - this->width()) /2, (QApplication::desktop()->height() - this->height()) /2);

    connect(msg_tip_, SIGNAL(activateFriendRequestDlg(ShareQQMsgPtr)), this, SLOT(openFriendRequestDlg(ShareQQMsgPtr)));
    connect(msg_tip_, SIGNAL(activateGroupRequestDlg(ShareQQMsgPtr)), this, SLOT(openGroupRequestDlg(ShareQQMsgPtr)));
    
    connect(msg_center_, SIGNAL(buddiesStateChangeMsgArrive(QString,FriendStatus, ClientType)),  this, SLOT(changeFriendStatus(QString,FriendStatus, ClientType)));
    connect(msg_center_, SIGNAL(newUnProcessMsg(ShareQQMsgPtr)), msg_tip_, SLOT(pushMsg(ShareQQMsgPtr)));

    connect(ui->tv_friendlist_, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openChatDlgByDoubleClick(QModelIndex)));
    connect(ui->lv_grouplist_, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openChatDlgByDoubleClick(QModelIndex)));
    connect(ui->lv_recentlist_, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openChatDlgByDoubleClick(QModelIndex)));
    connect(ui->cb_status_, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMyStatus(int)));
    connect(ui->pb_mainmenu_, SIGNAL(clicked()), this, SLOT(openMainMenu()));

    convertor_.addUinNameMap(QQSettings::instance()->currLoginInfo().id, tr("you"));
    msg_tip_->setConvertor(&convertor_);

    if (QFile::exists(QQSettings::configDir() + "/qqgroupdb"))
    {
        QFile::remove(QQSettings::configDir() + "/qqgroupdb");
    }

    QSettings setting(QQSettings::configDir() + "/options.ini", QSettings::IniFormat);
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

    initialize();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::slot_logout()
{
    this->hide();
    SystemTray::instance()->hide();

    if ( msg_tip_ )
        msg_tip_->deleteLater();
    msg_tip_ = NULL;

    if ( chat_manager_ )
        chat_manager_->deleteLater();
    chat_manager_ = NULL;

    if ( msg_center_ )
        msg_center_->deleteLater();
    msg_center_ = NULL;

    if ( poll_thread_ )
    {
        poll_thread_->terminate();
        poll_thread_->deleteLater();
        poll_thread_ = NULL;
    }
    if (parse_thread_)
    {
        parse_thread_->terminate();
        parse_thread_->deleteLater();
        parse_thread_ = NULL;
    }

    main_http_->close();
    emit sig_logout();
}

void MainWindow::setMute(bool mute)
{
    QSettings setting("options.ini", QSettings::IniFormat);
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

    QQSettings::instance()->currLoginInfo().status = ui->cb_status_->itemData(idx).value<FriendStatus>();
}

void MainWindow::changeFriendStatus(QString id, FriendStatus status, ClientType client_type)
{
    friend_model_->changeFriendStatus(id, status, client_type);

    ui->recents->setUpdatesEnabled(false);
    ui->recents->setUpdatesEnabled(true);
}

void MainWindow::closeEvent(QCloseEvent *)
{
    if (QFile::exists(QQSettings::configDir() + "/qqgroupdb"))
    {
        QFile::remove(QQSettings::configDir() + "/qqgroupdb");
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
    QByteArray friends_info = main_http_->readAll();

    friend_model_ = new FriendItemModel(this);
    friend_model_->parse(friends_info, &convertor_);

    ui->tv_friendlist_->setModel(friend_model_);

    getGroupList();
}

void MainWindow::getSingleLongNick()
{
    QByteArray result = QQItemInfoHelper::getSingleLongNick(QQSettings::instance()->currLoginInfo().id);
    result = result.mid(result.indexOf("\r\n\r\n")+4);
    Json::Reader reader;
    Json::Value root;

    if (reader.parse(QString(result).toStdString(), root, false))
    {
         ui->le_mood_->setText(QString::fromStdString(root["result"][0]["lnick"].asString()));
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
    qDebug()<<"group list"<<groups_info<<endl;
    group_model_ = new GroupItemModel(this);
    group_model_->parse(groups_info, &convertor_);

    msg_tip_->setConvertor(&convertor_);
    ui->lv_grouplist_->setModel(group_model_);

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

    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(QString(online_buddies).toStdString(), root, false))
    {
        return;
    }

    const Json::Value result = root["result"];

    for (unsigned int i = 0; i < result.size(); ++i)
    {
        QString id = QString::number(result[i]["uin"].asLargestInt());
        QString status = QString::fromStdString(result[i]["status"].asString());
        ClientType client_type = (ClientType)result[i]["client_type"].asInt();
        friend_model_->changeFriendStatus(id, QQUtility::stringToStatus(status), client_type);
    }

    poll_thread_ = new PollThread();
    parse_thread_ = new ParseThread();
    connect(poll_thread_, SIGNAL(signalNewMsgArrive(QByteArray)), parse_thread_, SLOT(pushRawMsg(QByteArray)));
    connect(parse_thread_, SIGNAL(parseDone(ShareQQMsgPtr)), msg_center_, SLOT(pushMsg(ShareQQMsgPtr)));
    poll_thread_->start();
    parse_thread_->start();
    msg_center_->start();
}

void MainWindow::getPersonalFace()
{
    QString avatar_path =   QQAvatarRequester::requestOne(QQAvatarRequester::getTypeNumber(QQItem::kFriend), QQSettings::instance()->currLoginInfo().id, QQGlobal::tempPath());
    QQSettings::instance()->currLoginInfo().avatar_path = avatar_path;

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
    QByteArray result = QQItemInfoHelper::getFriendInfo2(QQSettings::instance()->currLoginInfo().id);
    result = result.mid(result.indexOf("\r\n\r\n")+4);
    Json::Reader reader;
    Json::Value root;

    if (reader.parse(QString(result).toStdString(), root, false))
    {
        QQSettings::instance()->currLoginInfo().name = QString::fromStdString(root["result"]["nick"].asString());
        ui->lbl_name_->setText(QQSettings::instance()->loginName());
        convertor_.addUinNameMap(QQSettings::instance()->loginId(), QQSettings::instance()->loginName());
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

    recent_model_ = new RecentListItemModel(friend_model_, group_model_, this);
    connect(msg_center_, SIGNAL(groupChatMsgArrive(QString)), recent_model_, SLOT(improveItem(QString)));
    connect(msg_center_, SIGNAL(friendChatMsgArrive(QString)), recent_model_, SLOT(improveItem(QString)));
    recent_model_->parse(recent_list);
    ui->lv_recentlist_->setModel(recent_model_);

    getOnlineBuddy();
}

void MainWindow::initialize()
{
    getPersonalInfo();
    getPersonalFace();
    getSingleLongNick();
    
    getFriendList();

    chat_manager_ = new ChatManager(this, &convertor_),
    msg_tip_->setMainWindow(this);
}

void MainWindow::createTray()
{
    SystemTray *tray_icon = NULL;
    tray_icon = SystemTray::instance();
    tray_icon->setMsgTip(msg_tip_);

    connect(msg_tip_, SIGNAL(newUncheckMsgArrived()), tray_icon,  SLOT(slotNewUncheckMsgArrived()));
    connect(msg_tip_, SIGNAL(noUncheckMsg()), tray_icon,  SLOT(slotUncheckMsgEmpty()));

    tray_menu_ = new TrayMenu();

    minimize_ = new TrayMenuItem(tr("Minimize"));
    connect(minimize_, SIGNAL(triggered()), this, SLOT(hide()));

    restore_ = new TrayMenuItem(tr("Restore"));
    connect(restore_, SIGNAL(triggered()), this, SLOT(showNormal()));

    quit_ = new TrayMenuItem(tr("Quit"));
    connect(quit_, SIGNAL(triggered()), qApp, SLOT(quit()));

    logout_ = new TrayMenuItem(tr("Logout"));
    connect(logout_, SIGNAL(triggered()), this, SLOT(slot_logout()));

    tray_menu_->appendMenuItem(minimize_);
    tray_menu_->appendMenuItem(restore_);
    tray_menu_->appendMenuItem(logout_);
    tray_menu_->appendMenuItem(quit_);

    tray_icon->setIcon(QQSkinEngine::instance()->getSkinRes("app_icon"));
    tray_icon->setContextMenu(tray_menu_);

    tray_icon->show();
}

void MainWindow::openFirstChatDlg()
{
    msg_tip_->activatedChat(0);
}

QString MainWindow::getStatusByIndex(int idx) const
{
    switch (ui->cb_status_->itemData(idx).value<FriendStatus>())
    {
    case kOnline:
        return "online";
    case kCallMe:
        return "callme";
    case kAway:
        return "away";
    case kBusy:
        return "busy";
    case kSilent:
        return "silent";
    case kHidden:
        return "hidden";
    case kOffline:
        return "offline";
    default:
        break;
    }
    return "offline";
}

void MainWindow::setupLoginStatus()
{
    ui->cb_status_->addItem(QIcon(QQSkinEngine::instance()->getSkinRes("status_online")), tr("Online"), QVariant::fromValue<FriendStatus>(kOnline));
    ui->cb_status_->addItem(QIcon(QQSkinEngine::instance()->getSkinRes("status_qme")), tr("CallMe"), QVariant::fromValue<FriendStatus>(kCallMe));
    ui->cb_status_->addItem(QIcon(QQSkinEngine::instance()->getSkinRes("status_away")), tr("Away"), QVariant::fromValue<FriendStatus>(kAway));
    ui->cb_status_->addItem(QIcon(QQSkinEngine::instance()->getSkinRes("status_busy")), tr("Busy"), QVariant::fromValue<FriendStatus>(kBusy));
    ui->cb_status_->addItem(QIcon(QQSkinEngine::instance()->getSkinRes("status_mute")), tr("Silent"), QVariant::fromValue<FriendStatus>(kSilent));
    ui->cb_status_->addItem(QIcon(QQSkinEngine::instance()->getSkinRes("status_hidden")), tr("Hidden"), QVariant::fromValue<FriendStatus>(kHidden));
    ui->cb_status_->addItem(QIcon(QQSkinEngine::instance()->getSkinRes("status_offline")), tr("Offline"), QVariant::fromValue<FriendStatus>(kOffline));

    int status_idx = getStatusIndex(QQSettings::instance()->currLoginInfo().status);

    ui->cb_status_->setCurrentIndex(status_idx);
}

int MainWindow::getStatusIndex(FriendStatus status)
{
    for (int i = 0; i < ui->cb_status_->count(); ++i)
    {
        if (ui->cb_status_->itemData(i).value<FriendStatus>() == status)
            return i;
    }
    return -1;
}

void MainWindow::openChatDlgByDoubleClick(const QModelIndex& index)
{
    QQItem *item =  static_cast<QQItem*>(index.internalPointer());

    if (item->type() == QQItem::kFriend)
    {
        chat_manager_->openFriendChatDlg(item->id());
    }
    else if (item->type() == QQItem::kGroup)
    {
        chat_manager_->openGroupChatDlg(item->id(), item->gCode());
    }
}

void MainWindow::openFriendRequestDlg(ShareQQMsgPtr msg)
{
    FriendRequestDlg dlg(msg, (FriendItemModel*)friend_model_);
    if (dlg.exec() == QDialog::Accepted)
    {

    }
    else
    {

    }
}

void MainWindow::openGroupRequestDlg(ShareQQMsgPtr msg)
{
    GroupRequestDlg dlg(msg, (FriendItemModel*)friend_model_, (GroupItemModel*)group_model_);
    if (dlg.exec() == QDialog::Accepted)
    {

    }
    else
    {

    }
}

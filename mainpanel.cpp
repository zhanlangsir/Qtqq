#include "mainpanel.h"
#include "ui_qqmainpanel.h"

#include <QDesktopWidget>
#include <QHttp>
#include <QDebug>
#include <QTreeWidgetItem>
#include <QSemaphore>
#include <QDateTime>
#include <assert.h>
#include <QFile>
#include <QSettings>

#include "qqfriendrequestdlg.h"
#include "qqutility.h"
#include "frienditemmodel.h"
#include "groupitemmodel.h"
#include "qqgrouprequestdlg.h"
#include "qqiteminfohelper.h"
#include "qqavatarrequester.h"
#include "types.h"
#include "include/json/json.h"
#include "qqfriendchatdlg.h"
#include "qqgroupchatdlg.h"
#include "networkhelper.h"
#include "qqmsgcenter.h"
#include "qqmsgtip.h"
#include "qqpollthread.h"
#include "qqparsethread.h"

QQMainPanel::QQMainPanel(FriendInfo user_info, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QQMainPanel),
    main_http_(new QHttp),
    curr_user_info_(user_info),
    message_queue_(new QQueue<QByteArray>()),
    msg_tip_(new QQMsgTip(this)),
    msg_center_(new QQMsgCenter(msg_tip_))
{
    ui->setupUi(this);

    connect(msg_tip_, SIGNAL(activatedChatDlg(QQMsg::MsgType, QString, QString)), this, SLOT(openChatDlg(QQMsg::MsgType,QString, QString)));
    connect(msg_tip_, SIGNAL(activateFriendRequestDlg(QQMsg*)), this, SLOT(openFriendRequestDlg(QQMsg*)));
    connect(msg_tip_, SIGNAL(activateGroupRequestDlg(QQMsg*)), this, SLOT(openGroupRequestDlg(QQMsg*)));
    
    connect(msg_center_, SIGNAL(buddiesStateChangeMsgArrive(QString,FriendStatus, ClientType)),  this, SLOT(changeFriendStatus(QString,FriendStatus, ClientType)));
    connect(msg_center_, SIGNAL(groupChatMsgArrive(const QQGroupChatMsg*)), this, SLOT(changeRecentList(const QQGroupChatMsg*)));
    connect(msg_center_, SIGNAL(friendChatMsgArrive(const QQChatMsg*)), this, SLOT(changeRecentList(const QQChatMsg*)));

    connect(ui->tv_friendlist_, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openChatDlgByDoubleClick(QModelIndex)));
    connect(ui->lv_grouplist_, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openChatDlgByDoubleClick(QModelIndex)));
    connect(ui->lv_recentlist_, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openChatDlgByDoubleClick(QModelIndex)));
    connect(ui->cb_status_, SIGNAL(currentIndexChanged(int)), this, SLOT(changeMyStatus(int)));
    connect(ui->pb_mainmenu_, SIGNAL(clicked()), this, SLOT(openMainMenu()));

    convertor_.addUinNameMap(user_info.id(), tr("you"));
    msg_tip_->setConvertor(&convertor_);

    setWindowTitle(curr_user_info_.name());

    move((QApplication::desktop()->width() - this->width()) /2, (QApplication::desktop()->height() - this->height()) /2);

    if (QFile::exists("qqgroupdb"))
    {
        QFile::remove("qqgroupdb");
    }

    createActions();
    createTray();
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));

    setupLoginStatus();

    qRegisterMetaType<ClientType>("ClientType");

    //读取配置文件,根据配置文件设置好菜单选项
    QSettings setting("options.ini", QSettings::IniFormat);
    main_menu_ = new QMenu(this);
    act_mute_ = new QAction(tr("Mute"), main_menu_);
    act_mute_->setCheckable(true);
    act_mute_->setChecked(setting.value("mute").toBool());

    connect(act_mute_, SIGNAL(toggled(bool)), this, SLOT(setMute(bool)));

    main_menu_->addAction(act_mute_);
}

void QQMainPanel::setMute(bool mute)
{
    QSettings setting("options.ini", QSettings::IniFormat);
    setting.setValue("mute", mute);
}

void QQMainPanel::openMainMenu()
{
    main_menu_->popup(QCursor::pos());
}

QQMainPanel::~QQMainPanel()
{  
    delete trayIcon;
    trayIcon = NULL;
    main_http_->close();
    delete ui;

    poll_thread_->wait();
}

void QQMainPanel::changeMyStatus(int idx)
{
    QString change_status_url = "/channel/change_status2?newstatus=" + getStatusByIndex(idx) + 
        "&clientid=5412354841&psessionid=" + CaptchaInfo::singleton()->psessionid() + 
        "&t=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());

    QHttpRequestHeader header("GET", change_status_url);
    header.addValue("Host", "d.web2.qq.com");
    NetWorkHelper::setDefaultHeaderValue(header);
    header.addValue("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002&callback=1");
    header.addValue("Cookie", CaptchaInfo::singleton()->cookie());

    main_http_->setHost("d.web2.qq.com");
    main_http_->request(header);
}

void QQMainPanel::changeFriendStatus(QString id, FriendStatus status, ClientType client_type)
{
    friend_model_->changeFriendStatus(id, status, client_type);

    ui->recents->setUpdatesEnabled(false);
    ui->recents->setUpdatesEnabled(true);
}

void QQMainPanel::changeRecentList(const QQChatMsg *msg)
{
    QQItem *item = findRecentListItemById(msg->talkTo());
    if (!item)
    {
        item = friend_model_->find(msg->talkTo());
        if (!item) return;

        QQItem *recent_list_item = item->shallowCopy();
        recent_list_item->set_parent(recent_list_root_);
        recent_list_root_->append(recent_list_item);
    }
    else
    {
        int idx = recent_list_root_->indexOf(item);
        recent_list_root_->children_.remove(idx);
        recent_list_root_->children_.push_front(item);
    }
    ui->recents->setUpdatesEnabled(false);
    ui->recents->setUpdatesEnabled(true);
}

void QQMainPanel::changeRecentList(const QQGroupChatMsg *msg)
{
    QQItem *item = findRecentListItemById(msg->talkTo());
    if (!item)
    {
        item = group_model_->find(msg->talkTo());
        if (!item) return;

        QQItem *recent_list_item = item->shallowCopy();
        recent_list_item->set_parent(recent_list_root_);
        recent_list_root_->append(recent_list_item);
    }
    else
    {
        int idx = recent_list_root_->indexOf(item);
        recent_list_root_->children_.remove(idx);
        recent_list_root_->children_.push_front(item);
    }
    ui->recents->setUpdatesEnabled(false);
    ui->recents->setUpdatesEnabled(true);
}

void QQMainPanel::trayActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        if(this->isVisible())
            hide();
        else
            showNormal();
        break;
    default:
        ;
    }
}

void QQMainPanel::closeEvent(QCloseEvent *)
{
    QQChatDlg *dlg = NULL;
    foreach(dlg, opening_chatdlg_)
    {
        dlg->close();
    }

    if (QFile::exists("qqgroupdb"))
    {
        QFile::remove("qqgroupdb");
    }
}

void QQMainPanel::getFriendList()
{
    QString get_friendlist_url = "/api/get_user_friends2";
    QString msg_content = "r={\"h\":\"hello\",\"vfwebqq\":\"" + CaptchaInfo::singleton()->vfwebqq() + "\"}";

    QHttpRequestHeader header("POST", get_friendlist_url);
    header.addValue("Host", "s.web2.qq.com");
    NetWorkHelper::setDefaultHeaderValue(header);
    header.addValue("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001&callback=1&id=1");
    header.addValue("Cookie", CaptchaInfo::singleton()->cookie());
    header.setContentType("application/x-www-form-urlencoded");
    header.setContentLength(msg_content.length());

    main_http_->setHost("s.web2.qq.com");
    connect(main_http_, SIGNAL(done(bool)), this, SLOT(getFriendListDone(bool)));
    main_http_->request(header, msg_content.toAscii());
}

void QQMainPanel::getFriendListDone(bool err)
{
    Q_UNUSED(err)
    disconnect(main_http_, SIGNAL(done(bool)), this, SLOT(getFriendListDone(bool)));
    QByteArray friends_info = main_http_->readAll();

    friend_model_ = new FriendItemModel(this);
    friend_model_->parse(friends_info, &convertor_);

    ui->tv_friendlist_->setModel(friend_model_);

    //initrialiteΪQHttp첽ģ޷ȷһȵ
    getGroupList();
}

void QQMainPanel::getSingleLongNick()
{
    QByteArray result = QQItemInfoHelper::getSingleLongNick(curr_user_info_.id());
    result = result.mid(result.indexOf("\r\n\r\n")+4);  //??????
    Json::Reader reader;
    Json::Value root;

    if (reader.parse(QString(result).toStdString(), root, false))
    {
         ui->le_mood_->setText(QString::fromStdString(root["result"][0]["lnick"].asString()));
    }
}

void QQMainPanel::getGroupList()
{
    QString get_grouplist_url = "/api/get_group_name_list_mask2";
    QString msg_content = "r={\"h\":\"hello\",\"vfwebqq\":\"" + CaptchaInfo::singleton()->vfwebqq() + "\"}";
    QHttpRequestHeader header("POST", get_grouplist_url);
    header.addValue("Host", "s.web2.qq.com");
    NetWorkHelper::setDefaultHeaderValue(header);
    header.addValue("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001&callback=1&id=1");
    header.addValue("Cookie", CaptchaInfo::singleton()->cookie());
    header.setContentType("application/x-www-form-urlencoded");
    header.setContentLength(msg_content.length());

    main_http_->setHost("s.web2.qq.com");
    connect(main_http_, SIGNAL(done(bool)), this, SLOT(getGroupListDone(bool)));
    main_http_->request(header, msg_content.toAscii());
}

void QQMainPanel::getGroupListDone(bool err)
{
    Q_UNUSED(err)
    disconnect(main_http_, SIGNAL(done(bool)), this, SLOT(getGroupListDone(bool)));
    QByteArray groups_info = main_http_->readAll();

    group_model_ = new GroupItemModel(this);
    group_model_->parse(groups_info, &convertor_);

    msg_tip_->setConvertor(&convertor_);
    ui->lv_grouplist_->setModel(group_model_);

    getRecentList();
}

void QQMainPanel::getOnlineBuddy()
{
    QString get_online_buddy = "/channel/get_online_buddies2?clientid=5412354841&psessionid=" + CaptchaInfo::singleton()->psessionid() + "&t=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    QHttpRequestHeader header("GET", get_online_buddy);
    header.addValue("Host", "d.web2.qq.com");
    NetWorkHelper::setDefaultHeaderValue(header);
    header.addValue("Referer", "http://d.web2.qq.com/proxy.html?v=2011033100");
    header.addValue("Cookie", CaptchaInfo::singleton()->cookie());

    main_http_->setHost("d.web2.qq.com");
    connect(main_http_, SIGNAL(done(bool)), this, SLOT(getOnlineBuddyDone(bool)));
    main_http_->request(header);
}

void QQMainPanel::getOnlineBuddyDone(bool err)
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

    //޸ߺ״̬ΪkOnline
    for (unsigned int i = 0; i < result.size(); ++i)
    {
        QString id = QString::number(result[i]["uin"].asLargestInt());
        QString status = QString::fromStdString(result[i]["status"].asString());
        ClientType client_type = (ClientType)result[i]["client_type"].asInt();
        friend_model_->changeFriendStatus(id, QQUtility::stringToStatus(status), client_type);
    }

    poll_thread_ = new QQPollThread();
    parse_thread_ = new QQParseThread();
    connect(poll_thread_, SIGNAL(signalNewMsgArrive(QByteArray)), parse_thread_, SLOT(pushRawMsg(QByteArray)));
    connect(parse_thread_, SIGNAL(parseDone(QQMsg*)), msg_center_, SLOT(pushMsg(QQMsg*)));
    poll_thread_->start();
    parse_thread_->start();
    msg_center_->start();
}

void QQMainPanel::getPersonalFace()
{
    QString avatar_path =    QQAvatarRequester::requestOne(QQAvatarRequester::getTypeNumber(QQItem::kFriend), curr_user_info_.id(),  "temp/avatar/");

    QPixmap pix(avatar_path);
    ui->lbl_avatar_->setPixmap(pix);
}

void QQMainPanel::getPersonalInfo()
{
    QByteArray result = QQItemInfoHelper::getFriendInfo2(curr_user_info_.id());
    result = result.mid(result.indexOf("\r\n\r\n")+4);  //??????
    Json::Reader reader;
    Json::Value root;

    if (reader.parse(QString(result).toStdString(), root, false))
    {
        curr_user_info_.set_name(QString::fromStdString(root["result"]["nick"].asString()));
        ui->lbl_name_->setText(curr_user_info_.name());
    }
}

void QQMainPanel::getRecentList()
{
    QString recent_list_url ="/channel/get_recent_list2";
    QString msg_content = "r={\"vfwebqq\":\"" + CaptchaInfo::singleton()->vfwebqq() +
            "\",\"clientid\":\"5412354841\",\"psessionid\":\"" + CaptchaInfo::singleton()->psessionid() +
            "\"}&cliendid=5412354841&psessionid=" + CaptchaInfo::singleton()->psessionid();

    QHttpRequestHeader header("POST", recent_list_url);
    header.addValue("Host", "d.web2.qq.com");
    NetWorkHelper::setDefaultHeaderValue(header);
    header.addValue("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002&callback=1");
    header.addValue("Cookie", CaptchaInfo::singleton()->cookie());
    header.setContentType("application/x-www-form-urlencoded");
    header.setContentLength(msg_content.length());

    main_http_->setHost("d.web2.qq.com");
    connect(main_http_, SIGNAL(done(bool)), this, SLOT(getRecentListDone(bool)));
    main_http_->request(header, msg_content.toAscii());
}

void QQMainPanel::getRecentListDone(bool err)
{
    Q_UNUSED(err)
    disconnect(main_http_, SIGNAL(done(bool)), this, SLOT(getRecentListDone(bool)));
    QByteArray recent_list = main_http_->readAll();

    QQItemModel *model = new QQItemModel(this);
    QQItem *root_item = new QQItem;
    parseRecentList(recent_list, root_item);
    recent_list_root_ = root_item;
    model->setRoot(root_item);

    ui->lv_recentlist_->setModel(model);

    getOnlineBuddy();
}

QQItem* QQMainPanel::findRecentListItemById(QString id)
{
    QQItem *item = NULL;
    foreach (item, recents_info_)
    {
        if (item->id() == id)
        {
            return item;
        }
    }
    return NULL;
}

void QQMainPanel::initialize()
{
    ui->lbl_name_->setText(curr_user_info_.name());
    getPersonalInfo();
    getPersonalFace();
    getSingleLongNick();
    
    getFriendList();
}

void QQMainPanel::parseRecentList(const QByteArray &str, QQItem *const root_item)
{
    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(QString(str).toStdString(), root, false))
    {
        return;
    }

    const Json::Value result = root["result"];

    for (unsigned int i = 0; i < result.size(); ++i)
    {
        QString id = QString::number(result[i]["uin"].asLargestInt());
        int type = result[i]["type"].asInt();

        QQItem *item = NULL;
        if (type == 1)
        {
            item = group_model_->find(id);
        }
        else if(type == 0)
        {
            item = friend_model_->find(id);
        }
        if (!item)
            continue;

        QQItem *recent_list_item =  item->shallowCopy();
        recent_list_item->set_parent(root_item);
        recents_info_.append(recent_list_item);
        root_item->append(recent_list_item);
    }
}

void QQMainPanel::createTray()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    QIcon icon(":/new/login/images/WebQQ.ico");
    trayIcon = new QQSystemTray(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(icon);
    setWindowIcon(icon);
    trayIcon->show();
}

void QQMainPanel::createActions()
{
    minimizeAction = new QAction(tr("Mi&nimize"), this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

    restoreAction = new QAction(tr("&Restore"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}

QString QQMainPanel::getStatusByIndex(int idx) const
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

void QQMainPanel::setupLoginStatus()
{
    ui->cb_status_->addItem(tr("Online"), QVariant::fromValue<FriendStatus>(kOnline));
    ui->cb_status_->addItem(tr("CallMe"), QVariant::fromValue<FriendStatus>(kCallMe));
    ui->cb_status_->addItem(tr("Away"), QVariant::fromValue<FriendStatus>(kAway));
    ui->cb_status_->addItem(tr("Busy"), QVariant::fromValue<FriendStatus>(kBusy));
    ui->cb_status_->addItem(tr("Silent"), QVariant::fromValue<FriendStatus>(kSilent));
    ui->cb_status_->addItem(tr("Hidden"), QVariant::fromValue<FriendStatus>(kHidden));
    ui->cb_status_->addItem(tr("Offline"), QVariant::fromValue<FriendStatus>(kOffline));

    int status_idx = getStatusIndex(curr_user_info_.status());

    ui->cb_status_->setCurrentIndex(status_idx);
}

int QQMainPanel::getStatusIndex(FriendStatus status)
{
    for (int i = 0; i < ui->cb_status_->count(); ++i)
    {
        if (ui->cb_status_->itemData(i).value<FriendStatus>() == status)
            return i;
    }
    return -1;
}

void QQMainPanel::openChatDlgByDoubleClick(const QModelIndex& index)
{
    QQItem *item =  static_cast<QQItem*>(index.internalPointer());

    if (item->type() == QQItem::kFriend)
    {
        openChatDlg(QQMsg::kFriend, item->id(), item->gCode());
    }
    else if (item->type() == QQItem::kGroup)
    {
        openChatDlg(QQMsg::kGroup, item->id(), item->gCode());
    }
    else
        return;
}

void QQMainPanel::openFriendRequestDlg(QQMsg *msg)
{
    QQFriendRequestDlg dlg(msg, (FriendItemModel*)friend_model_);
    if (dlg.exec() == QDialog::Accepted)
    {

    }
    else
    {

    }
}

void QQMainPanel::openGroupRequestDlg(QQMsg *msg)
{
    QQGroupRequestDlg dlg(msg, (FriendItemModel*)friend_model_, (GroupItemModel*)group_model_);
    if (dlg.exec() == QDialog::Accepted)
    {

    }
    else
    {

    }
}

void QQMainPanel::openChatDlg(QQMsg::MsgType type, QString id, QString gcode)
{
    QQChatDlg *chatdlg = NULL;
    foreach(chatdlg, opening_chatdlg_)
    {
        if (chatdlg->id() == id)
            return;
    }

    QQChatDlg *dlg = NULL;
    if (type == QQMsg::kFriend)
    {
        QString avatar_path = "";
        QQItem *item = friend_model_->find(id);

        if (item)
            avatar_path = item->avatarPath();

        dlg= new QQFriendChatDlg(id, convertor_.convert(id), curr_user_info_, avatar_path);
        connect(dlg, SIGNAL(chatFinish(QQChatDlg*)), this, SLOT(closeChatDlg(QQChatDlg*)));
        msg_center_->registerListener(dlg);
    }
    else //kGroup
    {
        QString avatar_path = "";
        QQItem *item = group_model_->find(id);

        if (item)
            avatar_path = item->avatarPath();


        dlg = new QQGroupChatDlg(id, convertor_.convert(id), gcode, curr_user_info_, avatar_path);
        connect(dlg, SIGNAL(chatFinish(QQChatDlg*)), this, SLOT(closeChatDlg(QQChatDlg*)));

        msg_center_->registerListener(dlg);
    }
    opening_chatdlg_.append(dlg);
    dlg->move((QApplication::desktop()->width() - dlg->width()) /2, (QApplication::desktop()->height() - dlg->height()) /2);

    msg_tip_->removeItem(id);
}

void QQMainPanel::closeChatDlg(QQChatDlg *listener)
{
    opening_chatdlg_.remove(opening_chatdlg_.indexOf(listener));
    msg_center_->removeListener(listener);
    listener->deleteLater();
}

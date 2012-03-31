#include "mainpanel.h"
#include "types.h"
#include "ui_qqmainpanel.h"
#include "include/json/json.h"
#include "qqitemmodel.h"
#include "qqfriendchatdlg.h"
#include "qqgroupchatdlg.h"
#include "qqmsg.h"
#include "networkhelper.h"

#include <QDesktopWidget>
#include <QHttp>
#include <QDebug>
#include <QTreeWidgetItem>
#include <QSemaphore>
#include <QDateTime>
#include <assert.h>
#include <QFile>

QQMainPanel::QQMainPanel(FriendInfo user_info, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QQMainPanel),
    main_http_(new QHttp),
    curr_user_info_(user_info),
    poll_semapore_(new QSemaphore),
    parse_semapore_(new QSemaphore),
    message_queue_(new QQueue<QByteArray>()),
    check_old_msg_lock(new QMutex),
    msg_tip_(new QQMsgTip(this)),
    msg_center_(new QQMsgCenter(msg_tip_, check_old_msg_lock, parse_semapore_))
{
    ui->setupUi(this);
    connect(msg_tip_, SIGNAL(activatedChatDlg(QQMsg::MsgType, QString)), this, SLOT(openChatDlg(QQMsg::MsgType,QString)));
    connect(ui->tv_friendlist_, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openChatDlgByDoubleClick(QModelIndex)));
    connect(ui->lv_grouplist_, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openChatDlgByDoubleClick(QModelIndex)));
    connect(ui->lv_recents_list_, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openChatDlgByDoubleClick(QModelIndex)));
    connect(msg_center_, SIGNAL(buddiesStateChangeMsgArrive(QString,FriendStatus)), this, SLOT(changeFriendStatus(QString,FriendStatus)));
    connect(msg_center_, SIGNAL(groupChatMsgArrive(const QQGroupChatMsg*)), this, SLOT(changeRecentList(const QQGroupChatMsg*)));
    connect(msg_center_, SIGNAL(friendChatMsgArrive(const QQChatMsg*)), this, SLOT(changeRecentList(const QQChatMsg*)));

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
}

QQMainPanel::~QQMainPanel()
{  
    delete trayIcon;
    trayIcon = NULL;
    main_http_->close();
    delete ui;
}

void QQMainPanel::changeFriendStatus(QString id, FriendStatus state)
{
    QQItem *item = findFriendItemById(id);
    if (!item)
    {
        return;
    }

    item->set_state(state);

    QQItem *category = item->parent();
    int idx = category->indexOf(item);
    category->children_.remove(idx);

    if (state == kOnline)
        category->children_.push_front(item);
    else
        category->children_.push_back(item);

    ui->friends->setUpdatesEnabled(false);
    ui->friends->setUpdatesEnabled(true);

    ui->recents->setUpdatesEnabled(false);
    ui->recents->setUpdatesEnabled(true);
}

void QQMainPanel::changeRecentList(const QQChatMsg *msg)
{
    QQItem *item = findRecentListItemById(msg->talkTo());
    if (!item)
    {
        item = findFriendItemById(msg->talkTo());
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
        item = findGroupItemById(msg->talkTo());
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

    QQItemModel *model = new QQItemModel(this);
    QQItem *root_item = new QQItem;
    parseFriendsInfo(friends_info, root_item);
    model->setRoot(root_item);

    ui->tv_friendlist_->setModel(model);

    //在这里而不在initrialite调用因为用QHttp请求是异步的，无法确定那一个请求结果会先到，会引起解析混乱
    getGroupList();
}

void QQMainPanel::getSingleLongNick()
{
    QString single_long_nick_url = "/api/get_single_long_nick2?tuin=" + curr_user_info_.id()+ "&vfwebqq=" + CaptchaInfo::singleton()->vfwebqq() + "&t=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());

    QHttpRequestHeader header("GET", single_long_nick_url);
    header.addValue("Host", "s.web2.qq.com");
    NetWorkHelper::setDefaultHeaderValue(header);
    header.addValue("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001&callback=1&id=1");
    header.addValue("Cookie", CaptchaInfo::singleton()->cookie());
    header.addValue("Connection", "keep-live");
    header.setContentType("utf-8");

    main_http_->setHost("s.web2.qq.com");
    connect(main_http_, SIGNAL(done(bool)), this, SLOT(getSingleLongNickDone(bool)));
    main_http_->request(header);
}

void QQMainPanel::getSingleLongNickDone(bool err)
{
    Q_UNUSED(err)
    disconnect(main_http_, SIGNAL(done(bool)), this, SLOT(getSingleLongNickDone(bool)));
    QByteArray array = main_http_->readAll();

    Json::Reader reader;
    Json::Value root;

    if (reader.parse(QString(array).toStdString(), root, false))
    {
        ui->le_mood_->setText(QString::fromStdString(root["result"][0]["lnick"].asString()));
    }

    getFriendList();
}

void QQMainPanel::getPersonalInfo()
{
    QString get_personalinfo_url = "/api/get_friend_info2?tuin="+ curr_user_info_.id() +"&verifysession=&code=&vfwebqq=" + CaptchaInfo::singleton()->vfwebqq() + "&t=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    QHttpRequestHeader header("GET", get_personalinfo_url);
    header.addValue("Host", "s.web2.qq.com");
    NetWorkHelper::setDefaultHeaderValue(header);
    header.addValue("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001&callback=1&id=1");
    header.addValue("Cookie", CaptchaInfo::singleton()->cookie());
    header.addValue("Connection", "keep-live");
    header.setContentType("utf-8");

    main_http_->setHost("s.web2.qq.com");
    connect(main_http_, SIGNAL(done(bool)), this, SLOT(getPersonalInfoDone(bool)));
    main_http_->request(header);
}

void QQMainPanel::getPersonalInfoDone(bool err)
{
    Q_UNUSED(err)
    disconnect(main_http_, SIGNAL(done(bool)), this, SLOT(getPersonalInfoDone(bool)));
    QByteArray array = main_http_->readAll();

    Json::Reader reader;
    Json::Value root;

    if (reader.parse(QString(array).toStdString(), root, false))
    {
        curr_user_info_.set_name(QString::fromStdString(root["result"]["nick"].asString()));
        ui->lbl_name_->setText(curr_user_info_.name());
    }

    getPersonalFace();
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

    QQItemModel *model = new QQItemModel(this);
    QQItem *root_item = new QQItem;
    parseGroupsInfo(groups_info, root_item);
    model->setRoot(root_item);

    msg_tip_->setConvertor(&convertor_);
    ui->lv_grouplist_->setModel(model);

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

    //修改在线好友状态为kOnline
    for (unsigned int i = 0; i < result.size(); ++i)
    {
        QString id = QString::number(result[i]["uin"].asLargestInt());
        changeFriendStatus(id, kOnline);
    }

    poll_thread_ = new QQPollThread(message_queue_, poll_semapore_);
    parse_thread_ = new QQParseThread(message_queue_, poll_semapore_,parse_semapore_);
    connect(parse_thread_, SIGNAL(parseDone(QQMsg*)), msg_center_, SLOT(pushMsg(QQMsg*)));
    poll_thread_->start();
    parse_thread_->start();
    msg_center_->start();
}

void QQMainPanel::getPersonalFace()
{
    QString get_face_url = "/cgi/svr/face/getface?cache=1&type=1&fid=0&uin="+ curr_user_info_.id() + "&vfwebqq=" + CaptchaInfo::singleton()->vfwebqq() + "&t=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    QHttpRequestHeader header("GET", get_face_url);
    header.addValue("Host", "face1.qun.qq.com");
    NetWorkHelper::setDefaultHeaderValue(header);
    header.addValue("Referer", "http://web2.qq.com/");
    header.addValue("Cookie", CaptchaInfo::singleton()->cookie());

    main_http_->setHost("face1.qun.qq.com");
    connect(main_http_, SIGNAL(done(bool)), this, SLOT(getPersonalFaceDone(bool)));
    main_http_->request(header);
}

void QQMainPanel::getPersonalFaceDone(bool err)
{
    Q_UNUSED(err)
    disconnect(main_http_, SIGNAL(done(bool)), this, SLOT(getPersonalFaceDone(bool)));
    QByteArray array = main_http_->readAll();

    QPixmap pix;
    pix.loadFromData(array);
    ui->lbl_avatar_->setPixmap(pix);

    getSingleLongNick();
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

    ui->lv_recents_list_->setModel(model);

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

QQItem* QQMainPanel::findFriendItemById(QString id)
{
    QQItem *item = NULL;
    foreach (item, friends_info_)
    {
        if (item->id() == id)
        {
            return item;
        }
    }
    return NULL;
}

QQItem* QQMainPanel::findGroupItemById(QString id)
{
    QQItem *item = NULL;
    foreach (item, groups_info_)
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

    /*
    poll_thread_ = new QQPollThread(cap_info_, message_queue_, poll_semapore_);
    parse_thread_ = new QQParseThread(message_queue_, poll_semapore_,parse_semapore_);
    connect(parse_thread_, SIGNAL(parseDone(QQMsg*)), msg_center_, SLOT(pushMsg(QQMsg*)));
    poll_thread_->start();
    parse_thread_->start();
    msg_center_->start();*/
}

void QQMainPanel::parseFriendsInfo(const QByteArray &str, QQItem *root_item)
{
    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(QString(str).toStdString(), root, false))
    {
        return;
    }

    const Json::Value category = root["result"]["categories"];

    ItemInfo *friend_item = new ItemInfo;
    friend_item->set_name(tr("My Friends"));
    QQItem *friend_cat = new QQItem(QQItem::kCategory, friend_item, root_item);

    root_item->append(friend_cat);

    QMap<int, int> index_map;
    for (unsigned int i = 0; i < category.size(); ++i)
    {
        index_map.insert(i + 1, category[i]["index"].asInt());

        ItemInfo *item = new ItemInfo;
        item->set_name(QString::fromStdString(category[i]["name"].asString()));
        QQItem *cat = new QQItem(QQItem::kCategory, item, root_item);
        root_item->append(cat);
    }

    ItemInfo *stranger_item = new ItemInfo;
    stranger_item->set_name(tr("Strangers"));
    QQItem *stranger_cat = new QQItem(QQItem::kCategory, stranger_item, root_item);
    root_item->append(stranger_cat);

    const Json::Value friends = root["result"]["friends"];
    const Json::Value info = root["result"]["info"];

    for (unsigned int i = 0; i < friends.size(); ++i)
    {
        QString name = QString::fromStdString(info[i]["nick"].asString());
        QString uin = QString::number(info[i]["uin"].asLargestInt());

        FriendInfo *item = new FriendInfo;
        item->set_name(name);
        item->set_id(uin);
        item->set_state(kLeave);
        int category_num = friends[i]["categories"].asInt();
        QQItem* parent = root_item->value(index_map.key(category_num));
        QQItem *myfriend = new QQItem(QQItem::kFriend, item, parent);

        friends_info_.append(myfriend);
        convertor_.addUinNameMap(uin, name);

        parent->append(myfriend);
    }
}

void QQMainPanel::parseRecentList(const QByteArray &str, QQItem *root_item)
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
            item = findGroupItemById(id);
        }
        else if(type == 0)
        {
            item = findFriendItemById(id);
        }
        if (!item)
            continue;

        QQItem *recent_list_item =  item->shallowCopy();
        recent_list_item->set_parent(root_item);
        recents_info_.append(recent_list_item);
        root_item->append(recent_list_item);
    }
}

void QQMainPanel::parseGroupsInfo(const QByteArray &str, QQItem *root_item)
{
    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(QString(str).toStdString(), root, false))
    {
        return;
    }

    const Json::Value result = root["result"];
    const Json::Value gnamelist = result["gnamelist"];

    for (unsigned int i = 0; i < gnamelist.size(); ++i)
    {  
        QString name = QString::fromStdString(gnamelist[i]["name"].asString());
        QString gid = QString::number(gnamelist[i]["gid"].asLargestInt());
        QString code = QString::number(gnamelist[i]["code"].asLargestInt());

        GroupInfo *info= new GroupInfo;
        info->set_name(name);
        info->set_id(gid);
        info->set_code(code);

        QQItem *group = new QQItem(QQItem::kGroup, info, root_item);

        groups_info_.append(group);

        convertor_.addUinNameMap(gid, name);;

        root_item->append(group);
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

void QQMainPanel::openChatDlgByDoubleClick(const QModelIndex& index)
{
    QQItem *item =  static_cast<QQItem*>(index.internalPointer());

    if (item->type() == QQItem::kFriend)
    {
        openChatDlg(QQMsg::kFriend, item->id());
    }
    else if (item->type() == QQItem::kGroup)
    {
        openChatDlg(QQMsg::kGroup, item->id());
    }
    else
        return;
}

void QQMainPanel::openChatDlg(QQMsg::MsgType type, QString id)
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
        dlg= new QQFriendChatDlg(id, convertor_.convert(id), curr_user_info_, this);
        connect(dlg, SIGNAL(chatFinish(QQChatDlg*)), this, SLOT(closeChatDlg(QQChatDlg*)));
        msg_center_->registerListener(dlg);
    }
    else //kGroup
    {
        QQItem *item = NULL;
        foreach(item, groups_info_)
        {
            if (item->id() == id)
                break; //跳出foreach,而不是if
        }
        const GroupInfo *info = static_cast<const GroupInfo*>(item->itemInfo());

        dlg = new QQGroupChatDlg(id, convertor_.convert(id), info->code(), curr_user_info_, this);
        connect(dlg, SIGNAL(chatFinish(QQChatDlg*)), this, SLOT(closeChatDlg(QQChatDlg*)));

        msg_center_->registerListener(dlg);
    }
    opening_chatdlg_.append(dlg);
    dlg->move((QApplication::desktop()->width() - dlg->width()) /2, (QApplication::desktop()->height() - dlg->height()) /2);
}

void QQMainPanel::closeChatDlg(QQChatDlg *listener)
{
    opening_chatdlg_.remove(opening_chatdlg_.indexOf(listener));
    msg_center_->removeListener(listener);
}

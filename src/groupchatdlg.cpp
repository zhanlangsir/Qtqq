#include "groupchatdlg.h"
#include "ui_groupchatdlg.h"

#include <QScrollBar>
#include <QDateTime>
#include <QHttpRequestHeader>
#include <QMouseEvent>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QRegExp>
#include <QTcpSocket>

#include <assert.h>

#include "jsoncpp/include/json.h"

#include "qqitemmodel.h"
#include "core/qqutility.h"
#include "core/groupimgloader.h"
#include "core/groupimgsender.h"
#include "core/qqskinengine.h"
#include "core/captchainfo.h"
#include "core/groupchatlog.h"
#include "core/chatmanager.h"
#include "core/sockethelper.h"
#include "frienditemmodel.h"
#include "mainwindow.h"

GroupChatDlg::GroupChatDlg(QString gid, QString name, QString group_code, QString avatar_path,
                           QString send_url, ChatManager *chat_manager, MainWindow *main_win, QWidget *parent) :
    QQChatDlg(gid, name, send_url, parent),
    ui(new Ui::GroupChatDlg()),
    group_code_(group_code),
    model_(NULL),
    avatar_path_(avatar_path),
    chat_manager_(chat_manager),
    main_win_(main_win)
{
   ui->setupUi(this);
   set_type(QQChatDlg::kGroup);

   initUi();  
   updateSkin();
   initConnections();

   te_input_.setFocus();
   getGfaceSig();
}

GroupChatDlg::~GroupChatDlg()
{
    if ( model_ )
    {
        delete model_;
        model_ = NULL;
    }

    disconnect();
    http_.disconnect(this);
    delete ui;
}

void GroupChatDlg::initUi()
{
    setWindowTitle(name_);
    ui->lbl_name_->setText(name_);

    if (avatar_path_.isEmpty())
        avatar_path_ = QQSkinEngine::instance()->getSkinRes("default_group_avatar");
    QFile file(avatar_path_);
    file.open(QIODevice::ReadOnly);
    QPixmap pix;
    pix.loadFromData(file.readAll());
    file.close();
    ui->lbl_avatar_->setPixmap(pix);

    ui->btn_send_key->setMenu(send_type_menu_);

    ui->splitter_left_->insertWidget(0, &msgbrowse_);
    ui->splitter_left_->setChildrenCollapsible(false);
    ui->v_layout_left_->insertWidget(1, &te_input_);
    ui->splitter_main->setChildrenCollapsible(false);
    ui->splitter_right->setChildrenCollapsible(false);

    //设置分割器大小
    QList<int> main_sizes;
    main_sizes.append(500);
    main_sizes.append(ui->splitter_right->midLineWidth());
    ui->splitter_main->setSizes(main_sizes);

    QList<int> left_sizes;
    left_sizes.append(500);
    left_sizes.append(ui->splitter_left_->midLineWidth());
    ui->splitter_left_->setSizes(left_sizes);

    QList<int> right_sizes;
    right_sizes.append(200);
    right_sizes.append(this->height());
    ui->splitter_right->setSizes(right_sizes);

    this->resize(600, 500);
}

void GroupChatDlg::initConnections()
{
   connect(ui->btn_send_img, SIGNAL(clicked(bool)), this, SLOT(openPathDialog(bool)));
   connect(ui->btn_send_msg, SIGNAL(clicked()), this, SLOT(sendMsg()));
   connect(ui->btn_qqface, SIGNAL(clicked()), this, SLOT(openQQFacePanel()));
   connect(ui->btn_close, SIGNAL(clicked()), this, SLOT(close()));
   connect(ui->btn_chat_log, SIGNAL(clicked()), this, SLOT(openChatLogWin()));
   connect(ui->lv_members_, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(openChatDlgByDoubleClicked(const QModelIndex &)));

   connect(&msgbrowse_, SIGNAL(senderLinkClicked(QString)), this, SLOT(openSessOrFriendChatDlg(QString)));
}

void GroupChatDlg::updateSkin()
{

}

void GroupChatDlg::closeEvent(QCloseEvent *)
{
   //saveGroupInfo();
    writeMemberInfoToSql();
    emit chatFinish(this);
}

/*
void GroupChatDlg::saveGroupInfo()
{
  writeMemberInfoToSql();
  
}
*/

void GroupChatDlg::openChatDlgByDoubleClicked(const QModelIndex &index)
{
    QQItem *item =  static_cast<QQItem*>(index.internalPointer());
    openSessOrFriendChatDlg(item->id());
}

void GroupChatDlg::openSessOrFriendChatDlg(QString id)
{
    if ( main_win_->friendModel()->find(id) )
        chat_manager_->openFriendChatDlg(id);
    else
    {
        msg_sig_ = getMsgSig(id);
        chat_manager_->openSessChatDlg(id, id_);
    }
}

QString GroupChatDlg::getMsgSig(QString to_id) const
{
    QString msg_sig_url = "/channel/get_c2cmsg_sig2?id="+ id_ +"&to_uin=" + to_id +
            "&service_type=0&clientid=5412354841&psessionid=" + CaptchaInfo::instance()->psessionid() +"&t=" + QString::number(QDateTime::currentMSecsSinceEpoch());

    Request req;
    req.create(kGet, msg_sig_url);
    req.addHeaderItem("Host", "d.web2.qq.com");
    req.addHeaderItem("Content-Type", "utf-8");
    req.addHeaderItem("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002");
    req.addHeaderItem("Cookie", CaptchaInfo::instance()->cookie());

    QTcpSocket fd;
    fd.connectToHost("d.web2.qq.com", 80);
    fd.write(req.toByteArray());

    QByteArray result;
    socketReceive(&fd, result);
    fd.close();

    int sig_s_idx = result.indexOf("value")+8;
    int sig_e_idx = result.indexOf('"', sig_s_idx);
    QString sig = result.mid(sig_s_idx, sig_e_idx - sig_s_idx);

    return sig;
}

ImgLoader *GroupChatDlg::getImgLoader() const
{
    return new GroupImgLoader();
}

QQChatLog *GroupChatDlg::getChatlog() const
{
    return new GroupChatLog(group_code_);
}

ImgSender* GroupChatDlg::getImgSender() const
{
    return new GroupImgSender();
}

void GroupChatDlg::getGfaceSig()
{
    bool need_create_table = true;
    {
        QStringList connection_names = QSqlDatabase::connectionNames();
        QSqlDatabase db;
        if (connection_names.isEmpty())
        {
            db = QSqlDatabase::addDatabase("QSQLITE");
            db.setDatabaseName("qqgroupdb");
        }
        else
        {
            db = QSqlDatabase::database(connection_names[0]);
        }

        if (!db.open())
            return;

        connection_name_ = db.connectionName();
        QSqlQuery query;
        query.exec("SELECT count(*) FROM sqlite_master WHERE type='table' and name='groupmemberinfo'");

        if (!query.first())
            qDebug()<<query.lastError()<<endl;

        bool exist_group_sig_table = query.value(0).toBool();

        if (exist_group_sig_table)
        {
            QSqlQuery query;
            QString command = "SELECT count(*) FROM groupsig WHERE gid == %1";
            query.exec(command.arg(id_));

            if (!query.first())
                qDebug()<<query.lastError()<<endl;

            int exist_record_count = query.value(0).toInt();

            if (exist_record_count != 0)
            {
                need_create_table = false;
                readSigFromSql();
            }
        }
    }
    if (need_create_table)
    {
        QString gface_sig_url = "/channel/get_gface_sig2?clientid=5412354841&psessionid="+CaptchaInfo::instance()->psessionid() +
                "&t="+QString::number(QDateTime::currentMSecsSinceEpoch());

        QHttpRequestHeader header;
        header.setRequest("GET", gface_sig_url);
        header.addValue("Host", "d.web2.qq.com");
        header.addValue("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002");
        header.addValue("Cookie", CaptchaInfo::instance()->cookie());

        http_.setHost("d.web2.qq.com");
        connect(&http_, SIGNAL(done(bool)), this, SLOT(getGfaceSigDone(bool)));

        http_.request(header);
    }
    else
        getGroupMemberList();
}

void GroupChatDlg::readSigFromSql()
{
    QSqlQuery query;
    QString read_command = "SELECT * FROM groupsig WHERE gid == %1";
    query.exec(read_command.arg(id_));

    while (query.next())
    {
        QString key = query.value(1).toString();
        QString sig = query.value(2).toString();

        gface_key_ = key;
        gface_sig_ = sig;
    }
}

void GroupChatDlg::getGfaceSigDone(bool err)
{
    Q_UNUSED(err)
    disconnect(&http_, SIGNAL(done(bool)), this, SLOT(getGfaceSigDone(bool)));

    QByteArray array = http_.readAll();

    int gface_key_idx = array.indexOf("gface_key")+12;
    int gface_key_end_idx = array.indexOf(",",gface_key_idx)-1;

    int gface_sig_idx = array.indexOf("gface_sig")+12;
    int gface_sig_end_idx = array.indexOf("}", gface_sig_idx)-1;

    gface_key_ = array.mid(gface_key_idx, gface_key_end_idx - gface_key_idx);
    gface_sig_ = array.mid(gface_sig_idx, gface_sig_end_idx - gface_sig_idx);

    {
    QSqlDatabase db = QSqlDatabase::database(connection_name_);

    if (!db.open())
        return;

    QSqlQuery query;
    QSqlDatabase::database().transaction();

    createSigSql();

    QString insert_command = "INSERT INTO groupsig VALUES (%1, '%2', '%3')";
    query.exec(insert_command.arg(id_).arg(gface_key_).arg(gface_sig_));

    QSqlDatabase::database().commit();
    }

   getGroupMemberList();
}

void GroupChatDlg::createSigSql()
{
    QSqlQuery query;

    query.exec("CREATE TABLE IF NOT EXISTS groupsig ("
        "gid INTERGER,"
        "key VARCHAR(15),"
        "Sig VARCHAR(50),"
        "PRIMARY KEY (gid))");

    if (query.lastError().isValid())
    {
        qDebug()<<query.lastError();
    }
}

void GroupChatDlg::parseGroupMemberList(const QByteArray &array)
{
    Json::Value root;
    Json::Reader reader;

    if (!reader.parse(QString(array).toStdString(), root, false))
        return;

    Json::Value members = root["result"]["minfo"];

    for (unsigned int i = 0; i < members.size(); ++i)
    {
        QString nick = QString::fromStdString(members[i]["nick"].asString());
        QString uin = QString::number(members[i]["uin"].asLargestInt());

        QQItem *info = new QQItem(QQItem::kFriend, nick, uin, model_->rootItem());
        info->set_status(kOffline);
        model_->insertItem(info);

        convertor_.addUinNameMap(uin, nick);
    }

    Json::Value mark_names = root["result"]["cards"];
    for (unsigned int i = 0; i < mark_names.size(); ++i)
    {
        QString uin = QString::number(mark_names[i]["muin"].asLargestInt());
        QString mark_name = QString::fromStdString(mark_names[i]["card"].asString());

        QQItem *item = model_->find(uin);
        assert(item);

        item->set_markName(mark_name);
    }

    Json::Value stats = root["result"]["stats"];
    for (unsigned int i = 0; i < stats.size(); ++i)
    {
        ClientType client_type = (ClientType)stats[i]["client_type"].asInt();

        FriendStatus stat = (FriendStatus)stats[i]["stat"].asInt();
        QString uin = QString::number(stats[i]["uin"].asLargestInt());

        QQItem *item  = model_->find(uin);
        item->set_status(stat);
        item->set_clientType(client_type);

        model_->improveItem(uin);
    }

    Json::Value ginfo = root["result"]["ginfo"];
    QString g_announcement = QString::fromStdString(ginfo["memo"].asString());
    ui->announcement->setPlainText(g_announcement);
}

void GroupChatDlg::readFromSql()
{
    QSqlQuery query;
    QString read_command = "SELECT * FROM groupmemberinfo WHERE groupmemberinfo.gid == %1";
    query.exec(read_command.arg(id_));
    
    model_ = new QQItemModel();
    model_->setIconSize(QSize(25, 25));

    while (query.next())
    {
        QString uin = query.value(0).toString();
        QString nick = query.value(2).toString();
        QString mark_name = query.value(3).toString();
        FriendStatus stat = (FriendStatus)query.value(4).toInt();
        QString avatar_path = query.value(5).toString();

        QQItem *info = new QQItem(QQItem::kFriend, nick, uin, model_->rootItem());
        info->set_markName(mark_name);
        info->set_status(stat);
        info->set_avatarPath(avatar_path);

        if (info->status() == kOffline)
            model_->pushBack(info);
        else
            model_->pushFront(info);

        convertor_.addUinNameMap(uin, nick);
    }

    ui->lv_members_->setModel(model_);

    QString read_memo_command = "SELECT * FROM groupinfo WHERE groupinfo.gid == %1";
    query.exec(read_memo_command.arg(id_));

    while ( query.next() )
    {
        QString memo = query.value(1).toString();
        ui->announcement->setPlainText(memo);
    }

    replaceUnconverId();
}

void GroupChatDlg::replaceUnconverId()
{
    foreach (QString id, unconvert_ids_)
    {
        msgbrowse_.replaceIdToName(id, convertor_.convert(id));
    }
}

void GroupChatDlg::createSql()
{
    QSqlQuery query;

    query.exec("CREATE TABLE IF NOT EXISTS groupmemberinfo ("
        "uin INTEGER,"
        "gid INTERGER,"
        "name VARCHAR(15),"
        "markname VARCHAR(25),"
        "status INTEGER,"
        "avatarpath VARCHAR(20),"
        "PRIMARY KEY (uin))");

    if (query.lastError().isValid())
    {
        qDebug()<<query.lastError();
    }

    query.exec("CREATE TABLE IF NOT EXISTS groupinfo ("
            "gid INTERGER,"
            "memo VARCHAR(100),"
            "PRIMARY KEY (gid))");

    if (query.lastError().isValid())
    {
        qDebug()<<query.lastError();
    }
}

void GroupChatDlg::writeMemberInfoToSql()
{
    if ( !model_ )
        return;

    {
        QStringList connection_names = QSqlDatabase::connectionNames();
        QSqlDatabase db;
        if (connection_names.isEmpty())
        {
            db = QSqlDatabase::addDatabase("QSQLITE");
            db.setDatabaseName("qqgroupdb");
        }
        else
        {
            db = QSqlDatabase::database(connection_names[0]);
        }

        if (!db.open())
            return;

        createSql();

        QSqlQuery query;

        query.exec("SELECT count(*) FROM groupmemberinfo WHERE gid == " + id_);

        if (!query.first())
            qDebug()<<query.lastError()<<endl;

        int exist_record_count = query.value(0).toInt();

        if ( !exist_record_count )
        {
            QSqlDatabase::database().transaction();
            for (int i = 0; i < model_->rootItem()->children_.count(); ++i)
            {
                QQItem *item = model_->rootItem()->children_[i];

                //uin, gid, name, mark name, status, avatar path
                QString insert_command = "INSERT INTO groupmemberinfo VALUES (%1, %2, '%3', '%4', %5, '%6')";
                query.exec(insert_command.arg(item->id()).arg(id_).arg(item->name()).arg(item->markName()).arg(item->status()).arg(item->avatarPath()));
            }
            QSqlDatabase::database().commit();
        }

        QString insert_command = "INSERT INTO groupinfo VALUES (%1, '%2')";
        query.exec(insert_command.arg(id_).arg(ui->announcement->toPlainText()));
    }
    QString name;{
        name = QSqlDatabase::database().connectionName();
        QSqlDatabase::database().close();
    }
    QSqlDatabase::removeDatabase(name);
}

QQItem *GroupChatDlg::findItemById(QString id) const
{
    return model_->find(id);
}

void GroupChatDlg::getInfoById(QString id, QString &name, QString &avatar_path, bool &ok) const
{
    if ( model_ )
    {
        QQItem *item =  model_->find(id);
        if ( item )
        {
            name = item->markName();
            avatar_path = item->avatarPath().isEmpty() ? QQSkinEngine::instance()->getSkinRes("default_friend_avatar") : item->avatarPath();
            ok = true;
            return;
        }
    }


    name = convertor_.convert(id);
    avatar_path = QQSkinEngine::instance()->getSkinRes("default_friend_avatar");
    ok = false;
}

void GroupChatDlg::getGroupMemberList()
{
    {
        QSqlDatabase db = QSqlDatabase::database(connection_name_);

        if (!db.open())
            return;

        QSqlQuery query;
        query.exec("SELECT count(*) FROM sqlite_master WHERE type='table' and name='groupmemberinfo'");

        if (!query.first())
            qDebug()<<query.lastError()<<endl;

        bool exist_group_member_table = query.value(0).toBool();

        bool need_create_table = true;
        if (exist_group_member_table)
        {
            query.exec("SELECT count(*) FROM groupmemberinfo WHERE gid == " + id_);

            if (!query.first())
                qDebug()<<query.lastError()<<endl;

            int exist_record_count = query.value(0).toInt();

            if (exist_record_count != 0)
            {
                need_create_table = false;
                readFromSql();
            }
        }
        if (need_create_table)
        {
            QString get_group_member_url = "/api/get_group_info_ext2?gcode=" + group_code_ + "&vfwebqq=" +
                    CaptchaInfo::instance()->vfwebqq() + "&t="+ QString::number(QDateTime::currentMSecsSinceEpoch());

            QHttpRequestHeader header("GET", get_group_member_url);
            header.addValue("Host", "s.web2.qq.com");
            header.addValue("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001");
            header.addValue("Cookie", CaptchaInfo::instance()->cookie());

            http_.setHost("s.web2.qq.com");
            connect(&http_, SIGNAL(done(bool)), this, SLOT(getGroupMemberListDone(bool)));

            http_.request(header);
        }
    }

    QString name;{
        name = QSqlDatabase::database().connectionName();
        QSqlDatabase::database().close();
    }
   QSqlDatabase::removeDatabase(name);
}

void GroupChatDlg::getGroupMemberListDone(bool err)
{
    Q_UNUSED(err)
    disconnect(&http_, SIGNAL(done(bool)), this, SLOT(getGroupMemberListDone(bool)));

    QByteArray groups_member_info = http_.readAll();
    http_.close();

    model_ = new QQItemModel();
    model_->setIconSize(QSize(25, 25));
    parseGroupMemberList(groups_member_info);

    ui->lv_members_->setModel(model_);

   replaceUnconverId();
}

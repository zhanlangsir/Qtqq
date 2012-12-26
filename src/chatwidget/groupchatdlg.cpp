#include "groupchatdlg.h"
#include "ui_groupchatdlg.h"

#include <assert.h>

#include <QScrollBar>
#include <QDateTime>
#include <QHttpRequestHeader>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QRegExp>
#include <QTcpSocket>
#include <QThreadPool>


#include "json/json.h"

#include "core/qqutility.h"
#include "core/groupimgloader.h"
#include "core/groupimgsender.h"
#include "skinengine/qqskinengine.h"
#include "core/captchainfo.h"
#include "core/tasks.h"
#include "core/groupchatlog.h"
#include "core/sockethelper.h"
#include "chatwidget/chatdlg_manager.h"
#include "roster/roster.h"
#include "frienditemmodel.h"
#include "qqitemmodel.h"

GroupChatDlg::GroupChatDlg(Group *group, ChatDlgType type, QWidget *parent) :
    QQChatDlg(group, type, parent),
    ui(new Ui::GroupChatDlg()),
    model_(NULL)
{
   ui->setupUi(this);

   initUi();  
   updateSkin();
   initConnections();

   send_url_ =  "/channel/send_qun_msg2";

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
    setWindowTitle(talkable_->name());
    ui->lbl_name_->setText(talkable_->name());

	QPixmap pix = talkable_->icon();
	if ( !pix.isNull() )
		ui->lbl_avatar_->setPixmap(pix);
	else
		ui->lbl_avatar_->setPixmap(QPixmap(QQSkinEngine::instance()->skinRes("default_group_avatar")));

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
	connect(talkable_, SIGNAL(sigDataChanged(QVariant, TalkableDataRole)), this, SLOT(onTalkableDataChanged(QVariant, TalkableDataRole)));

   connect(ui->btn_send_img, SIGNAL(clicked(bool)), this, SLOT(openPathDialog(bool)));
   connect(ui->btn_send_msg, SIGNAL(clicked()), this, SLOT(sendMsg())); connect(ui->btn_qqface, SIGNAL(clicked()), this, SLOT(openQQFacePanel()));
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
    writeMemberInfoToSql();
    emit chatFinish(this);
}

void GroupChatDlg::openChatDlgByDoubleClicked(const QModelIndex &index)
{
    QQItem *item =  static_cast<QQItem*>(index.internalPointer());
    openSessOrFriendChatDlg(item->id());
}

void GroupChatDlg::openSessOrFriendChatDlg(QString id)
{
	Roster *roster = Roster::instance();
	Contact *contact = roster->contact(id);
    if ( contact )
        ChatDlgManager::instance()->openFriendChatDlg(id);
    else
    {
		msg_sig_ = getMsgSig(talkable_->id(), id);
		ChatDlgManager::instance()->openSessChatDlg(id, talkable_->id());
    }
}

QString GroupChatDlg::getMsgSig(QString gid,  QString to_id)
{
	QString msg_sig_url = "/channel/get_c2cmsg_sig2?id="+ gid +"&to_uin=" + to_id +
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
    return new GroupChatLog(talkable_->gcode());
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
            db.setDatabaseName(QQGlobal::configDir() + "/qqgroupdb");
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
            query.exec(command.arg(talkable_->id()));

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
    query.exec(read_command.arg(talkable_->id()));

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
    query.exec(insert_command.arg(talkable_->id()).arg(gface_key_).arg(gface_sig_));

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

    Json::Value mark_names = root["result"]["cards"];
    QHash <QString, QString> uin_marknames;
    for (unsigned int i = 0; i < mark_names.size(); ++i)
    {
        QString uin = QString::number(mark_names[i]["muin"].asLargestInt());
        QString mark_name = QString::fromStdString(mark_names[i]["card"].asString());

        uin_marknames.insert(uin, mark_name);
    }

    Json::Value members = root["result"]["minfo"];
    for (unsigned int i = 0; i < members.size(); ++i)
    {
        QString nick = QString::fromStdString(members[i]["nick"].asString());
        QString uin = QString::number(members[i]["uin"].asLargestInt());

        QQItem *item = new QQItem(QQItem::kFriend, nick, uin, model_->rootItem());
        item->set_markName(uin_marknames.value(uin, ""));
        item->set_status(CS_Offline);
        model_->insertItem(item);

        convertor_.addUinNameMap(uin, nick);

        //thread pool, have some problem
        //GetAvatarTask *task = new GetAvatarTask(item, model_);
        //QThreadPool::globalInstance()->start(task);
    }


    Json::Value stats = root["result"]["stats"];
    for (unsigned int i = 0; i < stats.size(); ++i)
    {
        ContactClientType client_type = (ContactClientType)stats[i]["client_type"].asInt();

        ContactStatus stat = (ContactStatus)stats[i]["stat"].asInt();
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
    query.exec(read_command.arg(talkable_->id()));
    
    model_ = new QQItemModel();
    model_->setIconSize(QSize(25, 25));

    int i = 0;
    while (query.next())
    {
        QString uin = query.value(0).toString();
        QString nick = query.value(2).toString();
        QString mark_name = query.value(3).toString();
        ContactStatus stat = (ContactStatus)query.value(4).toInt();
        QString avatar_path = query.value(5).toString();

        QQItem *info = new QQItem(QQItem::kFriend, nick, uin, model_->rootItem());
        info->set_markName(mark_name);
        info->set_status(stat);
        info->set_avatarPath(avatar_path);

        if (info->status() == CS_Offline)
            model_->pushBack(info);
        else
            model_->pushFront(info);

        convertor_.addUinNameMap(uin, nick);
    }

    ui->lv_members_->setModel(model_);

    QString read_memo_command = "SELECT * FROM groupinfo WHERE groupinfo.gid == %1";
    query.exec(read_memo_command.arg(talkable_->id()));

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
        QQItem *item = findItemById(id);
        if ( item != NULL )
            msgbrowse_.replaceIdToName(id, item->markName());
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
        "PRIMARY KEY (uin, gid))");

    if (query.lastError().isValid())
    {
        qDebug() << query.lastError();
    }

    query.exec("CREATE TABLE IF NOT EXISTS groupinfo ("
            "gid INTERGER,"
            "memo VARCHAR(100),"
            "PRIMARY KEY (gid))");

    if (query.lastError().isValid())
    {
        qDebug() << query.lastError();
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
            db.setDatabaseName(QQGlobal::configDir() + "/qqgroupdb");
        }
        else
        {
            db = QSqlDatabase::database(connection_names[0]);
        }

        if (!db.open())
            return;

        createSql();

        QSqlQuery query;

        query.exec("SELECT count(*) FROM groupmemberinfo WHERE gid == " + talkable_->id());

        if (!query.first())
            qDebug()<<query.lastError()<<endl;

        int exist_record_count = query.value(0).toInt();

        if ( !exist_record_count )
        {
            QString insert_command = "INSERT INTO groupmemberinfo VALUES (%1, %2, '%3', '%4', %5, '%6')";
            QSqlDatabase::database().transaction();
            for (int i = 0; i < model_->rootItem()->children_.count(); ++i)
            {
                QQItem *item = model_->rootItem()->children_[i];

                //uin, gid, name, mark name, status, avatar path
                query.exec(insert_command.arg(item->id()).arg(talkable_->id()).arg(item->name().replace('\'', "''")).arg(item->markName().replace('\'', "''")).arg(item->status()).arg(item->avatarPath()));

                if ( query.lastError().isValid() )
				{
                    qDebug() << "Write sql failed:" << item->name() << "  " <<  query.lastError().text() << endl;
				}
            }
            QSqlDatabase::database().commit();
        }

        QString insert_command = "INSERT INTO groupinfo VALUES (%1, '%2')";
        query.exec(insert_command.arg(talkable_->id()).arg(ui->announcement->toPlainText()));
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
            avatar_path = item->avatarPath().isEmpty() ? QQSkinEngine::instance()->skinRes("default_friend_avatar") : item->avatarPath();
            ok = true;
            return;
        }
    }


    name = convertor_.convert(id);
    avatar_path = QQSkinEngine::instance()->skinRes("default_friend_avatar");
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
            query.exec("SELECT count(*) FROM groupmemberinfo WHERE gid == " + talkable_->id());

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
            QString get_group_member_url = "/api/get_group_info_ext2?gcode=" + talkable_->gcode() + "&vfwebqq=" +
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

void GroupChatDlg::onTalkableDataChanged(QVariant data, TalkableDataRole role)
{
	switch ( role )
	{
		case TDR_Icon:
			ui->lbl_avatar_->setPixmap(data.value<QPixmap>());
			break;
		default:
			break;
	}
}


QString GroupChatDlg::chatItemToJson(const QVector<QQChatItem> &items)
{
	bool has_gface = false;
	QString json_msg;

	foreach ( const QQChatItem &item, items )
	{
		switch ( item.type() )
		{
			case QQChatItem::kWord:
				json_msg.append("\\\"" + item.content() + "\\\",");
				break;
			case QQChatItem::kQQFace:
				json_msg.append("[\\\"face\\\"," + item.content() + "],");
				break;
			case QQChatItem::kGroupChatImg:
				has_gface = false;
				json_msg.append("[\\\"cface\\\",\\\"group\\\",\\\"" + getUploadedFileInfo(item.content()).name + "\\\"],");
				break;
		}
	}

	json_msg = json_msg +
		"[\\\"font\\\",{\\\"name\\\":\\\"%E5%AE%8B%E4%BD%93\\\",\\\"size\\\":\\\"10\\\",\\\"style\\\":[0,0,0],\\\"color\\\":\\\"000000\\\"}]]\","
		"\"msg_id\":" + QString::number(msg_id_++) + ",\"clientid\":\"5412354841\","
		"\"psessionid\":\""+ CaptchaInfo::instance()->psessionid() +"\"}"
		"&clientid=5412354841&psessionid="+CaptchaInfo::instance()->psessionid();

	if (has_gface)
		json_msg = "r={\"group_uin\":" + id() +",\"group_code\":" + code() + "," + "\"key\":\"" + key()+ "\"," +
					"\"sig\":\"" + sig() + "\", \"content\":\"[" + json_msg;
	else
		json_msg = "r={\"group_uin\":" + id() + ",\"content\":\"[" + json_msg;


	return json_msg;
}

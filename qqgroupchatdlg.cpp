#include "qqgroupchatdlg.h"
#include "groupimgsender.h"
#include "ui_qqgroupchatdlg.h"
#include "include/json/json.h"

#include <QScrollBar>
#include <QDateTime>
#include <QHttpRequestHeader>
#include <QMouseEvent>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "qqitemmodel.h"
#include "qqutility.h"

QQGroupChatDlg::QQGroupChatDlg(QString gid, QString name, QString group_code, FriendInfo curr_user_info, QString avatar_path, QWidget *parent) :
    QQChatDlg(gid, name, curr_user_info, parent),
    ui(new Ui::QQGroupChatDlg()),
    group_code_(group_code)
{
   ui->setupUi(this);
   set_type(QQChatDlg::kGroup);

   ui->split_left_->insertWidget(0, &te_messages_);

   ui->spliter_main_->setChildrenCollapsible(false);

   QList<int> sizes;
   sizes.append(1000);
   sizes.append(ui->spliter_right_->midLineWidth());
   ui->spliter_main_->setSizes(sizes);

   ui->v_layout_left_->insertWidget(3, &te_input_);

   QScrollBar *bar = te_messages_.verticalScrollBar();
   connect(bar, SIGNAL(rangeChanged(int, int)), this, SLOT(silderToBottom(int, int)));
   connect(ui->tb_send_img_, SIGNAL(clicked(bool)), this, SLOT(openPathDialog(bool)));
   connect(ui->pb_send_msg_, SIGNAL(clicked()), this, SLOT(sendMsg()));
   connect(ui->tb_qqface_, SIGNAL(clicked()), this, SLOT(openQQFacePanel()));
   connect(ui->pb_close_, SIGNAL(clicked()), this, SLOT(close()));

   connect(ui->tb_close_, SIGNAL(clicked()), this, SLOT(close()));
   connect(ui->tb_mini_, SIGNAL(clicked()), this, SLOT(showMinimized()));

   ui->lbl_name_->setText(name_);
   setWindowOpacity(1);
   setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
   setWindowTitle(name_);

   send_url_ = "/channel/send_qun_msg2";

   if (avatar_path.isEmpty())
       avatar_path = "images/avatar/group.png";

   QFile file(avatar_path);
   file.open(QIODevice::ReadOnly);
   QPixmap pix;
   pix.loadFromData(file.readAll());
   file.close();

   ui->lbl_avatar_->setPixmap(pix);

   getGfaceSig();
}

QQGroupChatDlg::~QQGroupChatDlg()
{
    delete ui;
}

void QQGroupChatDlg::closeEvent(QCloseEvent *)
{
    writeMemberInfoToSql();
    emit chatFinish(this);
}

void QQGroupChatDlg::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
  QPoint origin_pos = this->pos();

  QPoint origin_mouse_pos = QCursor::pos();
  distance_pos_ = origin_mouse_pos - origin_pos;
}

void QQGroupChatDlg::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    if (distance_pos_.isNull())
    {
        return;
    }

    this->move(event->globalPos() - distance_pos_);
}

void QQGroupChatDlg::mouseReleaseEvent(QMouseEvent *)
{
    distance_pos_ = QPoint(0, 0);
}

ImgSender* QQGroupChatDlg::createImgSender()
{
    return new GroupImgSender();
}

void QQGroupChatDlg::getGfaceSig()
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
        QString gface_sig_url = "/channel/get_gface_sig2?clientid=5412354841&psessionid="+CaptchaInfo::singleton()->psessionid() +
                "&t="+QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());

        QHttpRequestHeader header;
        header.setRequest("GET", gface_sig_url);
        header.addValue("Host", "d.web2.qq.com");
        header.addValue("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002");
        header.addValue("Cookie", CaptchaInfo::singleton()->cookie());

        http_.setHost("d.web2.qq.com");
        connect(&http_, SIGNAL(done(bool)), this, SLOT(getGfaceSigDone(bool)));

        http_.request(header);
    }
    else
        getGroupMemberList();
}

void QQGroupChatDlg::readSigFromSql()
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

void QQGroupChatDlg::getGfaceSigDone(bool err)
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

void QQGroupChatDlg::createSigSql()
{
    QSqlQuery query;

    query.exec("CREATE TABLE IF NOT EXISTS groupsig ("
        "gid INTERGER,"
        "key VARCHAR(15),"
        "sig VARCHAR(50),"
        "PRIMARY KEY (gid))");

    if (query.lastError().isValid())
    {
        qDebug()<<query.lastError();
    }
    else
    {
        qDebug()<<"create groupsig db success"<<endl;
    }
}

void QQGroupChatDlg::parseGroupMemberList(const QByteArray &array, QQItem *const root_item)
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

        FriendInfo *info = new FriendInfo();
        info->set_name(nick);
        info->set_id(uin);
        info->set_status(kOffline);

        QQItem *member = new QQItem(QQItem::kFriend, info, root_item);
        root_item->append(member);

        convertor_.addUinNameMap(uin, nick);
    }

    Json::Value stats = root["result"]["stats"];

    for (unsigned int i = 0; i < stats.size(); ++i)
    {
        ClientType client_type = (ClientType)stats[i]["client_type"].asInt();

        FriendStatus stat = (FriendStatus)stats[i]["stat"].asInt();
        QString uin = QString::number(stats[i]["uin"].asLargestInt());

        QQItem *item  = NULL;
        int i;
        for (i = 0; i < root_item->children_.count(); ++i)
        {
            if (root_item->children_[i]->id() == uin)
            {
                item = root_item->children_[i];
                break;
            }
        }

        FriendInfo *info = static_cast<FriendInfo*>(item->itemInfo());
        info->set_status(stat);
        info->set_clientType(client_type);

        root_item->children_.remove(i);
        root_item->children_.push_front(item);
    }
}

void QQGroupChatDlg::createSql()
{
    QSqlQuery query;

    query.exec("CREATE TABLE IF NOT EXISTS groupmemberinfo ("
        "uin INTEGER,"
        "gid INTERGER,"
        "name VARCHAR(15),"
        "status INTEGER,"
        "avatarpath VARCHAR(20),"
        "PRIMARY KEY (uin))");

    if (query.lastError().isValid())
    {
        qDebug()<<query.lastError();
    }
    else
    {
        qDebug()<<"create groupmemberinfo db success"<<endl;
    }
}

void QQGroupChatDlg::readFromSql()
{
    QSqlQuery query;
    QString read_command = "SELECT * FROM groupmemberinfo WHERE groupmemberinfo.gid == %1";
    query.exec(read_command.arg(id_));
    
    QQItemModel *model = new QQItemModel(this);
    QQItem *root = new QQItem();
    while (query.next())
    {
        QString uin = query.value(0).toString();
        QString nick = query.value(2).toString();
        FriendStatus stat = (FriendStatus)query.value(3).toInt();
        QString avatar_path = query.value(4).toString();

        FriendInfo *info = new FriendInfo();
        info->set_name(nick);
        info->set_id(uin);
        info->set_status(stat);
        info->set_avatarPath(avatar_path);

        QQItem *member = new QQItem(QQItem::kFriend, info, root);
        if (info->status() == kOffline)
            root->children_.push_back(member);
        else
            root->children_.push_front(member);

        convertor_.addUinNameMap(uin, nick);
    }

    member_root_ = root;
    model->setRoot(root);
    ui->lv_members_->setModel(model);

    QString drop_command = "DROP TABLE IF EXISTS groupmemberinfo";
    query.exec(drop_command);

    replaceUnconverId();
}

void QQGroupChatDlg::replaceUnconverId()
{
    QString id;
    foreach (id, unconvert_ids_)
    {
        te_messages_.replaceIdToName(id, convertor_.convert(id));
    }
}

void QQGroupChatDlg::writeMemberInfoToSql()
{
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

        QSqlQuery query;
        QSqlDatabase::database().transaction();

        createSql();
        for (int i = 0; i < member_root_->children_.count(); ++i)
        {
            QQItem *item = member_root_->children_[i];

            QString insert_command = "INSERT INTO groupmemberinfo VALUES (%1, %2, '%3', %4, '%5')";
            query.exec(insert_command.arg(item->id()).arg(id_).arg(item->name()).arg(item->status()).arg(item->avatarPath()));
        }
        QSqlDatabase::database().commit();
    }
    QString name;{
        name = QSqlDatabase::database().connectionName();
        QSqlDatabase::database().close();
    }
    QSqlDatabase::removeDatabase(name);
}

QQItem *QQGroupChatDlg::findItemById(QString id) const
{
    QQItem *item  = NULL;
    foreach (item, member_root_->children_)
    {
        if (item->id() == id)
            return item;
    }
    return NULL;
}

void QQGroupChatDlg::getGroupMemberList()
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
                    CaptchaInfo::singleton()->vfwebqq() + "&t="+ QString::number(QDateTime::currentMSecsSinceEpoch());

            QHttpRequestHeader header("GET", get_group_member_url);
            header.addValue("Host", "s.web2.qq.com");
            header.addValue("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001");
            header.addValue("Cookie", CaptchaInfo::singleton()->cookie());

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

void QQGroupChatDlg::getGroupMemberListDone(bool err)
{
    Q_UNUSED(err)
    disconnect(&http_, SIGNAL(done(bool)), this, SLOT(getGroupMemberListDone(bool)));

    QByteArray groups_member_info = http_.readAll();
    http_.close();

    QQItemModel *model = new QQItemModel(this);
    QQItem *root_item = new QQItem;
    parseGroupMemberList(groups_member_info, root_item);
    member_root_ = root_item;
    model->setRoot(root_item);

    ui->lv_members_->setModel(model);

   replaceUnconverId();
}

QString QQGroupChatDlg::converToJson(const QString &raw_msg)
{
    bool has_gface = false;
    QString msg_template;

    int idx = raw_msg.indexOf("<p");
    int content_idx = raw_msg.indexOf(">", idx)+1;

    int content_end_idx = raw_msg.indexOf("</p>", content_idx);
    QString content = raw_msg.mid(content_idx, content_end_idx - content_idx);

    while (!content.isEmpty())
    {
        if (content[0] == '<')
        {
            int idx = content.indexOf("src") + 5;
            int end_idx = content.indexOf("\"", idx);
            QString src = content.mid(idx, end_idx - idx);

            if (src.contains("-"))
            {
                has_gface = true;
                msg_template.append("[\\\"cface\\\",\\\"group\\\",\\\"" + id_file_hash_[src].name_ + "\\\"],");
            }
            else
            {
                msg_template.append("[\\\"face\\\"," + src + "],");
            }

            content = content.mid(end_idx + 4);
        }
        else
        {
            int idx = content.indexOf("<");
            msg_template.append("\\\"" + content.mid(0, idx) + "\\\",");
            if (idx == -1)
                content = "";
            else
                content = content.mid(idx);
        }
    }

    msg_template = msg_template +
            "[\\\"font\\\",{\\\"name\\\":\\\"%E5%AE%8B%E4%BD%93\\\",\\\"size\\\":\\\"10\\\",\\\"style\\\":[0,0,0],\\\"color\\\":\\\"000000\\\"}]]\","
            "\"msg_id\":" + QString::number(msg_id_++) + ",\"clientid\":\"5412354841\","
            "\"psessionid\":\""+ CaptchaInfo::singleton()->psessionid() +"\"}"
            "&clientid=5412354841&psessionid="+CaptchaInfo::singleton()->psessionid();

    if (has_gface)
        msg_template = "r={\"group_uin\":" + id_ +",\"group_code\":" + group_code_ + "," + "\"key\":\"" + gface_key_ + "\"," +
            "\"sig\":\"" + gface_sig_ + "\", \"content\":\"[" + msg_template;
    else
        msg_template = "r={\"group_uin\":" + id_ + ",\"content\":\"[" + msg_template;

    return msg_template;
}

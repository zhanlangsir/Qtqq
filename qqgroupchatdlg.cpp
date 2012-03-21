#include "qqgroupchatdlg.h"
#include "groupimgsender.h"
#include "ui_qqgroupchatdlg.h"
#include "include/json/json.h"

#include <QScrollBar>
#include <QDateTime>
#include <QHttpRequestHeader>
#include <QMouseEvent>
#include <QDebug>

QQGroupChatDlg::QQGroupChatDlg(QString gid, QString name, QString group_code, FriendInfo curr_user_info, CaptchaInfo cap_info) : 
    QQChatDlg(gid, name, curr_user_info, cap_info),
    ui(new Ui::QQGroupChatDlg()),
    group_code_(group_code)
{
   ui->setupUi(this);

   ui->split_left_->insertWidget(0, &te_messages_);
   ui->spliter_main_->setStretchFactor(0, 1);
   ui->v_layout_left_->insertWidget(4, &te_input_);

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
   setWindowFlags(Qt::FramelessWindowHint);

   send_url_ = "/channel/send_qun_msg2";

   getGroupMemberList();
}

QQGroupChatDlg::~QQGroupChatDlg()
{
    delete ui;
}

void QQGroupChatDlg::mousePressEvent(QMouseEvent *event)
{
  QPoint origin_pos = this->pos();

  QPoint origin_mouse_pos = QCursor::pos();
  distance_pos_ = origin_mouse_pos - origin_pos;
}

void QQGroupChatDlg::mouseMoveEvent(QMouseEvent *event)
{
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
    QString gface_sig_url = "/channel/get_gface_sig2?clientid=5412354841&psessionid="+cap_info_.psessionid_ +"&t="+QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());

    QHttpRequestHeader header;
    header.setRequest("GET", gface_sig_url);
    header.addValue("Host", "d.web2.qq.com");
    header.addValue("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002");
    header.addValue("Cookie", cap_info_.cookie_);

    http_.setHost("d.web2.qq.com");
    connect(&http_, SIGNAL(done(bool)), this, SLOT(getGfaceSigDone(bool)));

    http_.request(header);
}

void QQGroupChatDlg::getGfaceSigDone(bool err)
{
    disconnect(&http_, SIGNAL(done(bool)), this, SLOT(getGfaceSigDone(bool)));

    QByteArray array = http_.readAll();
    http_.close();
    int gface_key_idx = array.indexOf("gface_key")+12;
    int gface_key_end_idx = array.indexOf(",",gface_key_idx)-1;

    int gface_sig_idx = array.indexOf("gface_sig")+12;
    int gface_sig_end_idx = array.indexOf("}", gface_sig_idx)-1;

    gface_key_ = array.mid(gface_key_idx, gface_key_end_idx - gface_key_idx);
    gface_sig_ = array.mid(gface_sig_idx, gface_sig_end_idx - gface_sig_idx);
}

void QQGroupChatDlg::getGroupMemberList()
{
    QString get_group_member_url = "/api/get_group_info_ext2?gcode=" + group_code_ + "&vfwebqq=" + cap_info_.vfwebqq_ + "&t="+ QString::number(QDateTime::currentMSecsSinceEpoch());

    QHttpRequestHeader header("GET", get_group_member_url);
    header.addValue("Host", "s.web2.qq.com");
    header.addValue("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001");
    header.addValue("Cookie", cap_info_.cookie_);

    http_.setHost("s.web2.qq.com");
    connect(&http_, SIGNAL(done(bool)), this, SLOT(getGroupMemberListDone(bool)));

    http_.request(header);
}

void QQGroupChatDlg::getGroupMemberListDone(bool err)
{
    disconnect(&http_, SIGNAL(done(bool)), this, SLOT(getGroupMemberListDone(bool)));
    QByteArray array = http_.readAll();
    http_.close();

    Json::Value root;
    Json::Reader reader;

    if (!reader.parse(QString(array).toStdString(), root, false))
        return;

    Json::Value members = root["result"]["minfo"];

    for (unsigned int i = 0; i < members.size(); ++i)
    {
        QString nick = QString::fromStdString(members[i]["nick"].asString());
        QString uin = QString::number(members[i]["uin"].asLargestInt());
        QListWidgetItem *item = new QListWidgetItem(nick, ui->lw_members_);
        item->setData(Qt::UserRole, uin);

        convertor_.addUinNameMap(uin, nick);

        item->setIcon(QIcon("1.bmp"));
    }
    getGfaceSig();
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
            "\"psessionid\":\""+ cap_info_.psessionid_ +"\"}"
            "&clientid=5412354841&psessionid="+cap_info_.psessionid_;

    if (has_gface)
        msg_template = "r={\"group_uin\":" + id_ +",\"group_code\":" + group_code_ + "," + "\"key\":\"" + gface_key_ + "\"," +
            "\"sig\":\"" + gface_sig_ + "\", \"content\":\"[" + msg_template;
    else
        msg_template = "r={\"group_uin\":" + id_ + ",\"content\":\"[" + msg_template;

    return msg_template;
}

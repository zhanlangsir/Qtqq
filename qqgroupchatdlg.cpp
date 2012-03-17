#include "qqgroupchatdlg.h"
#include "groupimgsender.h"
#include "ui_qqgroupchatdlg.h"
#include "include/json/json.h"

#include <QScrollBar>
#include <QDateTime>
#include <QTextEdit>
#include <QTextCursor>
#include <QDebug>

QQGroupChatDlg::QQGroupChatDlg(QString gid, QString name, QString group_code, FriendInfo curr_user_info, CaptchaInfo cap_info) : 
    QQChatDlg(gid, name, curr_user_info, cap_info),
    ui(new Ui::QQGroupChatDlg()),
    group_code_(group_code)
{
   ui->setupUi(this);

   ui->v_layout_left_->insertWidget(0, &te_messages_);
   ui->v_layout_left_->insertWidget(4, &te_input_);

   QScrollBar *bar = te_messages_.verticalScrollBar();
   connect(bar, SIGNAL(rangeChanged(int, int)), this, SLOT(silderToBottom(int, int)));
   connect(ui->tb_send_img_, SIGNAL(clicked(bool)), this, SLOT(openPathDialog(bool)));
   connect(ui->pb_send_msg_, SIGNAL(clicked()), this, SLOT(sendMsg()));
   connect(ui->tb_qqface_, SIGNAL(clicked()), this, SLOT(openQQFacePanel()));

   send_url_ = "/channel/send_qun_msg2";

   getGroupMemberList();
   getGfaceSig();
}

QQGroupChatDlg::~QQGroupChatDlg()
{
}

ImgSender* QQGroupChatDlg::createImgSender()
{
    return new GroupImgSender();
}

void QQGroupChatDlg::getGfaceSig()
{
    QString gface_sig_url = "/channel/get_gface_sig2?clientid=5412354841&psessionid="+cap_info_.psessionid_ +"&t="+QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());

    Request req;
    req.create(kGet, gface_sig_url);
    req.addHeaderItem("Host", "d.web2.qq.com");
    req.addHeaderItem("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002");
    req.addHeaderItem("Cookie", cap_info_.cookie_);
    fd_.connectToHost("d.web2.qq.com", 80);
    fd_.write(req.toByteArray());

    fd_.waitForReadyRead();
    QByteArray array;
    array.append(fd_.readAll());

    int gface_key_idx = array.indexOf("gface_key")+12;
    int gface_key_end_idx = array.indexOf(",",gface_key_idx)-1;

    int gface_sig_idx = array.indexOf("gface_sig")+12;
    int gface_sig_end_idx = array.indexOf("}", gface_sig_idx)-1;

    gface_key_ = array.mid(gface_key_idx, gface_key_end_idx - gface_key_idx);
    gface_sig_ = array.mid(gface_sig_idx, gface_sig_end_idx - gface_sig_idx);
    fd_.close();
}

void QQGroupChatDlg::getGroupMemberList()
{
    QString get_group_member_url = "/api/get_group_info_ext2?gcode=" + group_code_ + "&vfwebqq=" + cap_info_.vfwebqq_ + "&t="+ QString::number(QDateTime::currentMSecsSinceEpoch());

    Request req;
    req.create(kGet, get_group_member_url);
    req.addHeaderItem("Host", "s.web2.qq.com");
    req.addHeaderItem("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001");
    req.addHeaderItem("Cookie", cap_info_.cookie_);
    fd_.connectToHost("s.web2.qq.com", 80);
    fd_.write(req.toByteArray());

    fd_.waitForReadyRead();
    QByteArray array;
    array.append(fd_.readAll());

    int length_idx = array.indexOf("Content-Length") + 16;
    int length_end_idx = array.indexOf("\r\n", length_idx);

    QString length_str = array.mid(length_idx, length_end_idx - length_idx);
    int content_length = length_str.toInt();

    int img_idx = array.indexOf("\r\n\r\n")+4;
    array = array.mid(img_idx);

    while (array.length() < content_length)
    {
        fd_.waitForReadyRead();
        array.append(fd_.readAll());
    }

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

    fd_.close();
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

    msg_template = msg_template +  "\\\"\\\\n%E3%80%90%E6%8F%90%E7%A4%BA%EF%BC%9A%E6%AD%A4%E7%94%A8%E6%88%B7%E6%AD%A3%E5%9C%A8%E4%BD%BF%E7%94%A8Q%2B%20Web%EF%BC%9Ahttp:%2F%2Fweb.qq.com%2F%E3%80%91\\\","
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

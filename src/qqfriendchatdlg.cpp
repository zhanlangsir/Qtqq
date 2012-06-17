#include "qqfriendchatdlg.h"
#include "ui_qqfriendchatdlg.h"

#include <QScrollBar>
#include <QTextEdit>
#include <QTextCursor>
#include <QDebug>
#include <QMouseEvent>
#include <QFileDialog>
#include <QRegExp>

#include "core/friendimgsender.h"
#include "core/qqskinengine.h"
#include "core/captchainfo.h"
#include "core/friendchatlog.h"
#include "core/qqitem.h"

QQFriendChatDlg::QQFriendChatDlg(QString uin, QString from_name, QString avatar_path, QWidget *parent) :
    QQChatDlg(uin, from_name, parent),
    ui(new Ui::QQFriendChatDlg()),
    avatar_path_(avatar_path)
{
   ui->setupUi(this);
   resize(this->minimumSize());
   updateSkin();
   te_input_.setFocus();

   set_type(QQChatDlg::kFriend);

   ui->splitter_main->insertWidget(0, &msgbrowse_);
   ui->splitter_main->setChildrenCollapsible(false);

   ui->vlo_main->insertWidget(1, &te_input_);

   ui->btn_send_key->setMenu(send_type_menu_);

   QList<int> sizes;
   sizes.append(1000);
   sizes.append(ui->splitter_main->midLineWidth());
   ui->splitter_main->setSizes(sizes);

   //QScrollBar *bar = te_messages_.verticalScrollBar();
   //connect(bar, SIGNAL(rangeChanged(int, int)), this, SLOT(silderToBottom(int, int)));
   connect(ui->btn_send_img, SIGNAL(clicked(bool)), this, SLOT(openPathDialog(bool)));
   connect(ui->btn_send_msg, SIGNAL(clicked()), this, SLOT(sendMsg()));
   connect(ui->btn_qqface, SIGNAL(clicked()), this, SLOT(openQQFacePanel()));
   connect(ui->btn_close, SIGNAL(clicked()), this, SLOT(close()));
   connect(ui->btn_chat_log, SIGNAL(clicked()), this, SLOT(openChatLogWin()));

   setWindowTitle(name_);

   ui->lbl_name_->setText(name_);

   convertor_.addUinNameMap(id_, name_);
   send_url_ = "/channel/send_buddy_msg2";

   if (avatar_path.isEmpty())
       avatar_path = QQSkinEngine::instance()->getSkinRes("default_friend_avatar");

   QFile file(avatar_path);
   file.open(QIODevice::ReadOnly);
   QPixmap pix;
   pix.loadFromData(file.readAll());
   file.close();

   ui->lbl_avatar_->setPixmap(pix);
}

QQFriendChatDlg::~QQFriendChatDlg()
{
    delete ui;
}

void QQFriendChatDlg::updateSkin()
{

}

QString QQFriendChatDlg::converToJson(const QString &raw_msg)
{
    QString msg_template = "r={\"to\":" + id_ +",\"face\":525,"
            "\"content\":\"[";

    //提取<p>....</p>内容
    QRegExp p_reg("(<p.*</p>)");
    p_reg.setMinimal(true);

    int pos = 0;
    while ( (pos = p_reg.indexIn(raw_msg, pos)) != -1 )
    {
        QString content = p_reg.cap(0);
        while (!content.isEmpty())
        {
            if (content[0] == '<')
            {
                int match_end_idx = content.indexOf('>')+1;
                QString single_chat_item = content.mid(0, match_end_idx);

                int img_idx = single_chat_item.indexOf("src");
                if (img_idx != -1)
                {
                    img_idx += 5;
                    int img_end_idx = content.indexOf("\"", img_idx);
                    QString src = content.mid(img_idx, img_end_idx - img_idx);

                    if (src.contains(kQQFacePre))
                    {
                        msg_template.append("[\\\"face\\\"," + src.mid(kQQFacePre.length()) + "],");
                    }
                    else
                    {
                        msg_template.append("[\\\"offpic\\\",\\\""+ id_file_hash_[src].network_path + "\\\",\\\""+ id_file_hash_[src].name + "\\\"," + QString::number(id_file_hash_[src].size) + "],");
                    }
                }

                content = content.mid(match_end_idx);
            }
            else
            {
                int idx = content.indexOf("<");
                //&符号的html表示为&amp;而在json中为%26,所以要进行转换
                msg_template.append("\\\"" + content.mid(0,idx).replace("&amp;", "%26").replace('+', "%2B").replace(';', "%3B") + "\\\",");
                if (idx == -1)
                    content = "";
                else
                    content = content.mid(idx);
            }
        }

        msg_template.append("\\\"\\\\n\\\",");
        pos += p_reg.cap(0).length();
    }

    msg_template = msg_template +
            "[\\\"font\\\",{\\\"name\\\":\\\"%E5%AE%8B%E4%BD%93\\\",\\\"size\\\":\\\"10\\\",\\\"style\\\":[0,0,0],\\\"color\\\":\\\"000000\\\"}]]\","
            "\"msg_id\":" + QString::number(msg_id_++) + ",\"clientid\":\"5412354841\","
            "\"psessionid\":\""+ CaptchaInfo::singleton()->psessionid() +"\"}"
            "&clientid=5412354841&psessionid="+CaptchaInfo::singleton()->psessionid();
    //msg_template.replace("/", "%2F");
    return msg_template;
}

ImgSender* QQFriendChatDlg::getImgSender() const
{
    return new FriendImgSender();
}

void QQFriendChatDlg::getInfoById(QString id, QString &name, QString &avatar_path, bool &ok) const
{
    name = name_;
    avatar_path = avatar_path_.isEmpty() ? QQSkinEngine::instance()->getSkinRes("default_friend_avatar") : avatar_path_;
    ok = true;
}

QQChatLog *QQFriendChatDlg::getChatlog() const
{
    return new FriendChatLog(id());
}

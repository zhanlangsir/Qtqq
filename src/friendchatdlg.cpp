#include "friendchatdlg.h"
#include "ui_friendchatdlg.h"

#include <QScrollBar>
#include <QTextEdit>
#include <QTextCursor>
#include <QFileDialog>
#include <QRegExp>

#include "jsoncpp/include/json.h"

#include "core/friendimgsender.h"
#include "core/qqskinengine.h"
#include "core/captchainfo.h"
#include "core/friendchatlog.h"
#include "core/qqitem.h"
#include "qqiteminfohelper.h"

FriendChatDlg::FriendChatDlg(QString uin, QString from_name, QString avatar_path, QWidget *parent) :
    QQChatDlg(uin, from_name, parent),
    ui(new Ui::FriendChatDlg()),
    avatar_path_(avatar_path)
{
   ui->setupUi(this);

   set_type(QQChatDlg::kFriend);

   initUi();
   initConnections();
   updateSkin();

   send_url_ = "/channel/send_buddy_msg2";
   convertor_.addUinNameMap(id_, name_);

   te_input_.setFocus();
}

void FriendChatDlg::initUi()
{
    setWindowTitle(name_);
    ui->lbl_name_->setText(name_);

    ui->splitter_main->insertWidget(0, &msgbrowse_);
    ui->splitter_main->setChildrenCollapsible(false);
    ui->vlo_main->insertWidget(1, &te_input_);

    ui->btn_send_key->setMenu(send_type_menu_);

    QList<int> sizes;
    sizes.append(1000);
    sizes.append(ui->splitter_main->midLineWidth());
    ui->splitter_main->setSizes(sizes);

    if (avatar_path_.isEmpty())
        avatar_path_ = QQSkinEngine::instance()->getSkinRes("default_friend_avatar");
    QFile file(avatar_path_);
    file.open(QIODevice::ReadOnly);
    QPixmap pix;
    pix.loadFromData(file.readAll());
    file.close();
    ui->lbl_avatar_->setPixmap(pix);

    getSingleLongNick(id_);

    resize(this->minimumSize());
}


void FriendChatDlg::initConnections()
{
    connect(ui->btn_send_img, SIGNAL(clicked(bool)), this, SLOT(openPathDialog(bool)));
    connect(ui->btn_send_msg, SIGNAL(clicked()), this, SLOT(sendMsg()));
    connect(ui->btn_qqface, SIGNAL(clicked()), this, SLOT(openQQFacePanel()));
    connect(ui->btn_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->btn_chat_log, SIGNAL(clicked()), this, SLOT(openChatLogWin()));
}

FriendChatDlg::~FriendChatDlg()
{
    delete ui;
}

void FriendChatDlg::updateSkin()
{

}

void FriendChatDlg::getSingleLongNick(QString id)
{
    QByteArray result = QQItemInfoHelper::getSingleLongNick(id);
    result = result.mid(result.indexOf("\r\n\r\n")+4);

    Json::Reader reader;
    Json::Value root;

    if (reader.parse(QString(result).toStdString(), root, false))
    {
         ui->lbl_mood_->setText(QString::fromStdString(root["result"][0]["lnick"].asString()));
    }
}

ImgSender* FriendChatDlg::getImgSender() const
{
    return new FriendImgSender();
}

void FriendChatDlg::getInfoById(QString id, QString &name, QString &avatar_path, bool &ok) const
{
    Q_UNUSED(id)
    name = name_;
    avatar_path = avatar_path_.isEmpty() ? QQSkinEngine::instance()->getSkinRes("default_friend_avatar") : avatar_path_;
    ok = true;
}

QQChatLog *FriendChatDlg::getChatlog() const
{
    return new FriendChatLog(id());
}

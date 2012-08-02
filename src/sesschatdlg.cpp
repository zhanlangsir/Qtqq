#include "sesschatdlg.h"
#include "ui_sesschatdlg.h"

#include <QFile>

#include "core/qqskinengine.h"
#include "core/friendchatlog.h"

SessChatDlg::SessChatDlg(QString uin, QString from_name, QString avatar_path, QString group_name, QWidget *parent) :
    QQChatDlg(uin, from_name, parent),
    ui_(new Ui::SessChatDlg()),
    avatar_path_(avatar_path)
{
    ui_->setupUi(this);

    set_type(QQChatDlg::kSess);

    initUi(group_name);
    initConnections();
    updateSkin();

    send_url_ = "/channel/send_sess_msg2";

    convertor_.addUinNameMap(id_, name_);

    te_input_.setFocus();
}


void SessChatDlg::initUi(QString group_name)
{
    setWindowTitle(name_);
    ui_->lbl_name_->setText(name_);

    ui_->splitter_main->insertWidget(0, &msgbrowse_);
    ui_->splitter_main->setChildrenCollapsible(false);
    ui_->vlo_main->insertWidget(1, &te_input_);

    ui_->btn_send_key->setMenu(send_type_menu_);

    QList<int> sizes;
    sizes.append(1000);
    sizes.append(ui_->splitter_main->midLineWidth());
    ui_->splitter_main->setSizes(sizes);

    if (avatar_path_.isEmpty())
        avatar_path_ = QQSkinEngine::instance()->getSkinRes("default_friend_avatar");
    QFile file(avatar_path_);
    file.open(QIODevice::ReadOnly);
    QPixmap pix;
    pix.loadFromData(file.readAll());
    file.close();
    ui_->lbl_avatar_->setPixmap(pix);
    ui_->group_name->setText(group_name);

    resize(this->minimumSize());
}


void SessChatDlg::initConnections()
{
    connect(ui_->btn_send_msg, SIGNAL(clicked()), this, SLOT(sendMsg()));
    connect(ui_->btn_qqface, SIGNAL(clicked()), this, SLOT(openQQFacePanel()));
    connect(ui_->btn_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui_->btn_chat_log, SIGNAL(clicked()), this, SLOT(openChatLogWin()));
}

SessChatDlg::~SessChatDlg()
{
    delete ui_;
}

void SessChatDlg::updateSkin()
{

}

ImgSender* SessChatDlg::getImgSender() const
{
    return NULL;
}

void SessChatDlg::getInfoById(QString id, QString &name, QString &avatar_path, bool &ok) const
{
    Q_UNUSED(id)
    name = name_;
    avatar_path = avatar_path_.isEmpty() ? QQSkinEngine::instance()->getSkinRes("default_friend_avatar") : avatar_path_;
    ok = true;
}

QQChatLog *SessChatDlg::getChatlog() const
{
    return new FriendChatLog(id());
}

#include "sesschatdlg.h"
#include "ui_sesschatdlg.h"

#include <QFile>

#include "skinengine/qqskinengine.h"
#include "core/friendchatlog.h"

SessChatDlg::SessChatDlg(Contact *contact, QString group_name, ChatDlgType type, QWidget *parent) :
    QQChatDlg(contact, type, parent),
    ui_(new Ui::SessChatDlg())
{
    ui_->setupUi(this);

    initUi(group_name);
    initConnections();
    updateSkin();

    send_url_ = "/channel/send_sess_msg2";

    te_input_.setFocus();
}


void SessChatDlg::initUi(QString group_name)
{
    setWindowTitle(talkable_->name());
    ui_->lbl_name_->setText(talkable_->name());

    ui_->splitter_main->insertWidget(0, &msgbrowse_);
    ui_->splitter_main->setChildrenCollapsible(false);
    ui_->vlo_main->insertWidget(1, &te_input_);

    ui_->btn_send_key->setMenu(send_type_menu_);

    QList<int> sizes;
    sizes.append(1000);
    sizes.append(ui_->splitter_main->midLineWidth());
    ui_->splitter_main->setSizes(sizes);

    if (avatar_path_.isEmpty())
        avatar_path_ = QQSkinEngine::instance()->skinRes("default_friend_avatar");
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
    name = talkable_->name();
    avatar_path = avatar_path_.isEmpty() ? QQSkinEngine::instance()->skinRes("default_friend_avatar") : avatar_path_;
    ok = true;
}

QQChatLog *SessChatDlg::getChatlog() const
{
    return new FriendChatLog(id());
}

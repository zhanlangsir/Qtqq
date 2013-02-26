#include "sesschatdlg.h"
#include "ui_sesschatdlg.h"

#include <QFile>

#include "skinengine/qqskinengine.h"
#include "core/captchainfo.h"
#include "core/friendchatlog.h"
#include "core/talkable.h"
#include "chatwidget/groupchatdlg.h"
#include "protocol/qq_protocol.h"

SessChatDlg::SessChatDlg(Contact *contact, Group *group, ChatDlgType type, QWidget *parent) :
    QQChatDlg(contact, type, parent),
    ui_(new Ui::SessChatDlg()),
    group_(group)
{
    ui_->setupUi(this);

    initUi();
    initConnections();
    updateSkin();

    te_input_.setFocus();
}

void SessChatDlg::initUi()
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

	QPixmap pix = talkable_->avatar();
	if ( pix.isNull() )
        pix = QPixmap(QQSkinEngine::instance()->skinRes("default_friend_avatar"));

	ui_->lbl_avatar_->setPixmap(pix);
	ui_->group_name->setText(group_->name());

    resize(this->minimumSize());
}


void SessChatDlg::initConnections()
{
    connect(ui_->btn_send_msg, SIGNAL(clicked()), this, SLOT(sendMsg()));
    connect(ui_->btn_qqface, SIGNAL(clicked()), this, SLOT(openQQFacePanel()));
    connect(ui_->btn_close, SIGNAL(clicked()), this, SLOT(close()));
}

SessChatDlg::~SessChatDlg()
{
    delete talkable_;
    talkable_=NULL;
    delete ui_;
}

void SessChatDlg::updateSkin()
{

}

Contact *SessChatDlg::getSender(const QString &id) const
{
    return (Contact *)talkable_;
}

QQChatLog *SessChatDlg::getChatlog() const
{
    return new FriendChatLog(id());
}

void SessChatDlg::insertImage(const QString &file_path)
{
    te_input_.setToolTip(tr("Temporary session can't send image!"));
    te_input_.showToolTip();
}

void SessChatDlg::send(const QVector<QQChatItem> &msgs)
{
    Protocol::QQProtocol::instance()->sendMsg(talkable_, group_, msgs);
}

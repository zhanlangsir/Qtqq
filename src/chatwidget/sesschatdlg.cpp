#include "sesschatdlg.h"
#include "ui_sesschatdlg.h"

#include <QFile>

#include "skinengine/qqskinengine.h"
#include "core/captchainfo.h"
#include "core/friendchatlog.h"
#include "core/talkable.h"
#include "chatwidget/groupchatdlg.h"

SessChatDlg::SessChatDlg(Contact *contact, Group *group, ChatDlgType type, QWidget *parent) :
    QQChatDlg(contact, type, parent),
    ui_(new Ui::SessChatDlg())
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
    connect(ui_->btn_chat_log, SIGNAL(clicked()), this, SLOT(openChatLogWin()));
}

SessChatDlg::~SessChatDlg()
{
    delete ui_;
}

void SessChatDlg::updateSkin()
{

}


QQChatLog *SessChatDlg::getChatlog() const
{
    return new FriendChatLog(id());
}

/*
QString SessChatDlg::chatItemToJson(const QVector<QQChatItem> &items) 
{
    QString json_msg = "r={\"to\":" + id() + ",\"group_sig\":\"" + GroupChatDlg::getMsgSig(group_->id(), id()) + "\",\"face\":291,\"content\":\"[";

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
			case QQChatItem::kFriendOffpic:
				json_msg.append("[\\\"offpic\\\",\\\"" + getUploadedFileInfo(item.content()).network_path + "\\\",\\\"" + getUploadedFileInfo(item.content()).name + "\\\"," + QString::number(getUploadedFileInfo(item.content()).size) + "],");
				break;
		}
	}
	json_msg = json_msg +
        "[\\\"font\\\",{\\\"name\\\":\\\"%E5%AE%8B%E4%BD%93\\\",\\\"size\\\":\\\"10\\\",\\\"style\\\":[0,0,0],\\\"color\\\":\\\"000000\\\"}]]\","
        "\"msg_id\":" + QString::number(msg_id_++) + ",\"service_type\":0,\"clientid\":\"5412354841\","
        "\"psessionid\":\""+ CaptchaInfo::instance()->psessionid() +"\"}"
        "&clientid=5412354841&psessionid="+CaptchaInfo::instance()->psessionid();

	return json_msg;
}
*/

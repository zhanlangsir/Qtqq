#include "friendchatdlg.h"
#include "ui_friendchatdlg.h"

#include <QScrollBar>
#include <QTextEdit>
#include <QTextCursor>
#include <QFileDialog>
#include <QRegExp>

#include <json/json.h>

#include "core/friendimgsender.h"
#include "skinengine/qqskinengine.h"
#include "core/captchainfo.h"
#include "core/friendchatlog.h"
#include "core/qqitem.h"
#include "roster/roster.h"
#include "qqiteminfohelper.h"
#include "utils/icon_decorator.h"

FriendChatDlg::FriendChatDlg(Contact *contact, ChatDlgType type, QWidget *parent) :
    QQChatDlg(contact, type, parent),
    ui(new Ui::FriendChatDlg())
{
   ui->setupUi(this);

   initUi();
   initConnections();
   updateSkin();

   send_url_ = "/channel/send_buddy_msg2";

   te_input_.setFocus();
}

void FriendChatDlg::initUi()
{
    setWindowTitle(talkable_->name());
    ui->lbl_name_->setText(talkable_->name());

    ui->splitter_main->insertWidget(0, &msgbrowse_);
    ui->splitter_main->setChildrenCollapsible(false);
    ui->vlo_main->insertWidget(1, &te_input_);

    ui->btn_send_key->setMenu(send_type_menu_);

    QList<int> sizes;
    sizes.append(1000);
    sizes.append(ui->splitter_main->midLineWidth());
    ui->splitter_main->setSizes(sizes);

	QPixmap pix = talkable_->icon();
	if ( pix.isNull() )
	{
		QString avatar_path = QQSkinEngine::instance()->skinRes("default_friend_avatar");
		qDebug() << avatar_path << endl;
		QFile file(avatar_path);
		file.open(QIODevice::ReadOnly);
		pix.loadFromData(file.readAll());
		file.close();
	}

	QIcon icon;
	icon.addPixmap(pix);
	setWindowIcon(icon);

	IconDecorator::decorateIcon(talkable_->status(), pix);
	ui->lbl_avatar_->setPixmap(pix);

    getSingleLongNick(talkable_->id());

    resize(this->minimumSize());
}


void FriendChatDlg::initConnections()
{
	connect(talkable_, SIGNAL(sigDataChanged(QVariant, TalkableDataRole)), this, SLOT(onTalkableDataChanged(QVariant, TalkableDataRole)));
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
    name = talkable_->name();
    avatar_path =  QQSkinEngine::instance()->skinRes("default_friend_avatar");
    ok = true;
}

QQChatLog *FriendChatDlg::getChatlog() const
{
    return new FriendChatLog(id());
}

void FriendChatDlg::onTalkableDataChanged(QVariant data, TalkableDataRole role)
{
	switch ( role )
	{
		case TDR_Icon:
			ui->lbl_avatar_->setPixmap(data.value<QPixmap>());
			break;
		case TDR_Status:
			{
				ContactStatus status = data.value<ContactStatus>();
				QPixmap pix = talkable_->icon();
				if ( !pix.isNull() )
				{
					IconDecorator::decorateIcon(status, pix);
					ui->lbl_avatar_->setPixmap(pix);
				}
			}
			break;
		default:
			break;
	}
}


QString FriendChatDlg::chatItemToJson(const QVector<QQChatItem> &items)
{
	QString json_msg = "r={\"to\":" + id() +",\"face\":525,\"content\":\"[";

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
		"\"msg_id\":" + QString::number(msg_id_++) + ",\"clientid\":\"5412354841\","
		"\"psessionid\":\""+ CaptchaInfo::instance()->psessionid() +"\"}"
		"&clientid=5412354841&psessionid="+CaptchaInfo::instance()->psessionid();
	return json_msg;
}

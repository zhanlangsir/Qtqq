#include "friendchatdlg.h"
#include "ui_friendchatdlg.h"

#include "json/json.h"

#include "core/friendchatlog.h"
#include "skinengine/qqskinengine.h"
#include "utils/icon_decorator.h"
#include "qqiteminfohelper.h"

FriendChatDlg::FriendChatDlg(Contact *contact, ChatDlgType type, QWidget *parent) :
    QQChatDlg(contact, type, parent),
    ui(new Ui::FriendChatDlg())
{
   ui->setupUi(this);

   initUi();
   initConnections();
   updateSkin();

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

	QPixmap pix = talkable_->avatar();
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
	connect(talkable_, SIGNAL(dataChanged(QVariant, TalkableDataRole)), this, SLOT(onTalkableDataChanged(QVariant, TalkableDataRole)));
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

QQChatLog *FriendChatDlg::getChatlog() const
{
    return new FriendChatLog(id());
}

void FriendChatDlg::onTalkableDataChanged(QVariant data, TalkableDataRole role)
{
	switch ( role )
	{
		case TDR_Avatar:
			ui->lbl_avatar_->setPixmap(data.value<QPixmap>());
			break;
		case TDR_Status:
			{
				ContactStatus status = data.value<ContactStatus>();
				QPixmap pix = talkable_->avatar();
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

Contact *FriendChatDlg::getSender(const QString &id) const
{
    return (Contact *)talkable_;
}

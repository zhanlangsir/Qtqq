#include "friendchatdlg.h"
#include "ui_friendchatdlg.h"

#include <assert.h>

#include <QRegExp>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QClipboard>

#include "json/json.h"

#include "core/friendchatlog.h"
#include "skinengine/qqskinengine.h"
#include "utils/icon_decorator.h"
#include "msgprocessor/msg_processor.h"
#include "protocol/qq_protocol.h"
#include "file_transfer/file_transfer_manager.h"
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

    QMenu *send_file_menu = new QMenu(this);
    QAction *offfile_act = new QAction(tr("Send Offline File"), send_file_menu);
    QAction *file_act = new QAction(tr("Send File"), send_file_menu);
    connect(offfile_act, SIGNAL(triggered(bool)), this, SLOT(onOfffileSend()));
    connect(file_act, SIGNAL(triggered(bool)), this, SLOT(onFileSend()));
    send_file_menu->addAction(offfile_act);
    send_file_menu->addAction(file_act);

    ui->send_file_btn->setMenu(send_file_menu);

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
    connect(&msgbrowse_, SIGNAL(linkClicked(const QUrl &)), this, SLOT(onLinkClicked(const QUrl &)));
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
    Talkable *talkable = qobject_cast<Talkable *>(sender());
	switch ( role )
	{
		case TDR_Avatar:
			ui->lbl_avatar_->setPixmap(talkable->avatar());
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

void FriendChatDlg::showOtherMsg(ShareQQMsgPtr msg)
{
    onOffFileMsg(msg);
}

void FriendChatDlg::onOffFileMsg(ShareQQMsgPtr msg)
{
    assert(msg->type() == QQMsg::kOffFile); 

    const QQOffFileMsg *offfile_msg = static_cast<const QQOffFileMsg *>(msg.data());

    ShowOptions options; 
    options.type = MsgBrowse::kStatus;

    QDateTime date;
    date.setMSecsSinceEpoch(offfile_msg->time * 1000);
    options.time = date;
    options.sender_name = talkable_->name();

    QString link = "[offfile_link:http://%1:%2/%3?ver=2173&rkey=%4&range=0]";
    link = link.arg(offfile_msg->ip).arg(offfile_msg->port).arg(offfile_msg->name).arg(offfile_msg->rkey);
    QString message = tr("Peer send you a offline file: %1 ").arg(offfile_msg->name) + "<a href=\"" + link + "\">[" + tr("Copy download link") + "]</a>";    

    msgbrowse_.appendHtml(message, options);
}

void FriendChatDlg::onLinkClicked(const QUrl &url)
{
    QRegExp offfile_reg("\\[offfile_link:(.*)\\]");

    if ( offfile_reg.indexIn(url.toString()) != -1 )
    {
        QApplication::clipboard()->setText(offfile_reg.cap(1));
    }
}

void FriendChatDlg::onOfffileSend()
{
    QString file_path = QFileDialog::getOpenFileName(this, tr("select the file to send"), QDir::homePath(), QString());
    if ( file_path.isEmpty() )
        return;

    qDebug() << "Send Offline file: " << file_path << endl;

    FileTransferManager::instance()->sendOffFile(talkable_->id(), talkable_->markname(), file_path);
}

void FriendChatDlg::onFileSend()
{
    QString file_path = QFileDialog::getOpenFileName(this, tr("select the file to send"), QDir::homePath(), QString());
    if ( file_path.isEmpty() )
        return;

    qDebug() << "Send file: " << file_path << endl;

    FileTransferManager::instance()->sendFile(talkable_->id(), talkable_->markname(), file_path);
}

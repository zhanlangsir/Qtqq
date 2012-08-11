#include "msgtip.h"
#include "ui_msgtip.h"

#include <QVBoxLayout>
#include <QModelIndex>

#include "systemtray.h"
#include "mainwindow.h"
#include "frienditemmodel.h"
#include "groupitemmodel.h"
#include "core/qqitem.h"
#include "qqiteminfohelper.h"
#include <json/json.h>

#include "qqglobal.h"
#include "core/qqskinengine.h"

const static int NOTIFY_TIMEOUT_MS	= 5000;

MsgTip::MsgTip(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MsgTip)
{
    ui->setupUi(this);
    setWindowOpacity(1);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Dialog | Qt::FramelessWindowHint);
    qRegisterMetaType<SoundPlayer::SoundType>("SoundPlayer::SoundType");

    connect(ui->uncheckmsglist, SIGNAL(clicked(const QModelIndex&)), this, SLOT(slotActivated(const QModelIndex&)));
}

void MsgTip::pushMsg(ShareQQMsgPtr new_msg)
{
    if (new_msg->type() == QQMsg::kBuddiesStatusChange)
    {
        return;
    }

    if (new_msg->type() == QQMsg::kSystem || new_msg->type() == QQMsg::kSystemG)
    {
        bibibi(SoundPlayer::kSystem);
    }
    else
    {
        bibibi(SoundPlayer::kMsg);
    }

	showMessage(new_msg);

    lock.lock();
    for (int i = 0; i < ui->uncheckmsglist->count(); ++i)
    {
		QListWidgetItem *item = ui->uncheckmsglist->item(i);
        ShareQQMsgPtr msg = item->data(Qt::UserRole).value<ShareQQMsgPtr>();

        if (msg->talkTo() == new_msg->talkTo())
        {
			QString text = item->text();
			int idx = text.indexOf('\t', 0);
			// 仅一条未读
			if (idx == -1)
			{
				item->setText(text + "\t(2)");
			}
			else
			{
				QString number;
				QString newText = text;
				newText.truncate(idx);

				for(int i=idx+2; i<text.length(); ++i)
				{
					if (text.at(i) == ')')
						break;
					number.append(text.at(i));
				}

				uint count = number.toUInt();
				item->setText(newText + "\t(" + QString::number(count + 1) + ")");
			}
			// removeItem(msg->talkTo());
            lock.unlock();
            return;
        }
    }
    addItem(new_msg);
    lock.unlock();
 }

void MsgTip::addItem(ShareQQMsgPtr msg)
{
    //QQItem *info = main_win_->getFriendModel()->find(msg->talkTo());

    switch(msg->type())
    {
    case QQMsg::kSystem:
    {
        //QListWidgetItem *item = new QListWidgetItem("[" + convertor_->convert(msg->talkTo()) + "]" + "request to add you");
        QListWidgetItem *item = new QListWidgetItem("Friend request message!");
        item->setData(Qt::UserRole, QVariant::fromValue(msg));
        ui->uncheckmsglist->addItem(item);

        /*
            现在此方法无任何作用
            可以使用libnotify实现
        */
        // trayIcon->showMessage(uidImage(msg->sendUin()), "[" + convertor_->convert(msg->talkTo()) + "]" + "request to add you", msg->msg(),  200);
    }
        break;
    case QQMsg::kSystemG:
    {
        //QListWidgetItem *item = new QListWidgetItem("[" + convertor_->convert(msg->sendUin()) + "]" + "request to enter group [" + convertor_->convert(msg->sendUin()));
        QListWidgetItem *item = new QListWidgetItem("Group system message!");
        item->setData(Qt::UserRole, QVariant::fromValue(msg));
        ui->uncheckmsglist->addItem(item);

        // trayIcon->showMessage(uidImage(msg->sendUin()), "[" + convertor_->convert(msg->sendUin()) + "]" + "request to enter group [" + convertor_->convert(msg->sendUin()), msg->msg(),  200);
    }
        break;
    case QQMsg::kSess:
    {
        QString name;
        QString token;
        getStrangerInfo(msg->sendUin(), msg->talkTo(), name, token);
        QListWidgetItem *item = new QListWidgetItem(name);
        item->setData(Qt::UserRole, QVariant::fromValue(msg));
        ui->uncheckmsglist->addItem(item);

        // trayIcon->showMessage(uidImage(msg->sendUin()), "[" + convertor_->convert(msg->talkTo()) + "]" + " send message to " + "[" + tr("you") + "]", msg->msg(), 200);
        break;
    }
    case QQMsg::kFriend:
    {
        //QListWidgetItem *item = new QListWidgetItem("[" + convertor_->convert(msg->talkTo()) + "]" + " send message to " + "[" + tr("you") + "]");
        QListWidgetItem *item = new QListWidgetItem(convertor_->convert(msg->talkTo()));
        item->setData(Qt::UserRole, QVariant::fromValue(msg));
        ui->uncheckmsglist->addItem(item);

        // trayIcon->showMessage(uidImage(msg->sendUin()), "[" + convertor_->convert(msg->talkTo()) + "]" + " send message to " + "[" + tr("you") + "]", msg->msg(), 200);
        break;
    }
    case QQMsg::kGroup:
    {
        //QListWidgetItem *item = new QListWidgetItem("[" + convertor_->convert(msg->talkTo()) + "]" + " send message to " + "[" + tr("you") + "]");
        QListWidgetItem *item = new QListWidgetItem(convertor_->convert(msg->talkTo()));
        item->setData(Qt::UserRole, QVariant::fromValue(msg));
        ui->uncheckmsglist->addItem(item);

        // trayIcon->showMessage(uidImage(msg->talkTo()), "[" + convertor_->convert(msg->talkTo()) + "]" + " send message to " + "[" + tr("you") + "]", msg->msg(), 200);
        break;
    }
    }

    int item_height = 25;
    this->resize(this->width(), ui->uncheckmsglist->count() * item_height);
    emit newUncheckMsgArrived();
}

bool MsgTip::getStrangerInfo(QString id, QString gid, QString &name, QString &token) const
{
    QByteArray info = QQItemInfoHelper::getStrangetInfo2(id, gid);
	info = info.mid(info.indexOf("\r\n\r\n") + 4);

    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(QString(info).toStdString(), root, false))
    {
        return false;
    }

    name = QString::fromStdString(root["result"]["nick"].asString());
    token = QString::fromStdString(root["result"]["token"].asString());
}

void MsgTip::removeItem(QString id)
{
    for (int i = 0; i < ui->uncheckmsglist->count(); ++i)
    {
        ShareQQMsgPtr msg = ui->uncheckmsglist->item(i)->data(Qt::UserRole).value<ShareQQMsgPtr>();

        if (msg->talkTo() == id)
        {
            QListWidgetItem *wid = ui->uncheckmsglist->takeItem(i);
            delete wid;
            wid = NULL;
        }
    }

    if (ui->uncheckmsglist->count() == 0)
    {
        this->hide();
        emit noUncheckMsg();
    }
}

void MsgTip::show(QPoint pos)
{
    if ( ui->uncheckmsglist->count() == 0 )
        return;

    if ( !this->isVisible() )
        move(pos.x() - ui->uncheckmsglist->width() / 2, pos.y()-5);

    QWidget::show();
}

void MsgTip::leaveEvent(QEvent *)
{
    this->hide();
}

void MsgTip::bibibi(SoundPlayer::SoundType type)
{
    SoundPlayer::singleton()->play(type);
}

void MsgTip::slotActivated(const QModelIndex &index)
{
   activatedChat(index.row());
}

bool MsgTip::activatedChat(int i)
{
    if (ui->uncheckmsglist->count() < i+1)
		return false;

    QListWidgetItem *item = ui->uncheckmsglist->item(i);
    ShareQQMsgPtr msg = item->data(Qt::UserRole).value<ShareQQMsgPtr>();
    removeItem(msg->talkTo());

    switch (msg->type())
    {
    case QQMsg::kSess:
        main_win_->chatManager()->openSessChatDlg(msg->sendUin(), msg->gid());
        break;
    case QQMsg::kFriend:
        main_win_->chatManager()->openFriendChatDlg(msg->sendUin());
        break;
    case QQMsg::kGroup:
        main_win_->chatManager()->openGroupChatDlg(msg->talkTo(), msg->gCode());
        break;
    case QQMsg::kSystem:
        emit activateFriendRequestDlg(msg);
        break;
    case QQMsg::kSystemG:
        emit activateGroupRequestDlg(msg);
        break;
    }

	// 设置闪烁的头像为最后一个
	int count = ui->uncheckmsglist->count();
	if (count > 0)
	{
		QListWidgetItem *item = ui->uncheckmsglist->item(count - 1);
		ShareQQMsgPtr msg = item->data(Qt::UserRole).value<ShareQQMsgPtr>();

		SystemTray *trayIcon = SystemTray::instance();

		trayIcon->setIcon(getItemAvatar(msg->talkTo(), msg->type()));
	}

	return true;
}

QString MsgTip::getItemAvatar(const QString &uid, QQMsg::MsgType type)
{
    QString path;

    if ( type == QQMsg::kFriend )
    {
        path = main_win_->chatManager()->getFriendAvatarPath(uid);
        if ( path.isEmpty() )
            path =  QQSkinEngine::instance()->getSkinRes("default_friend_avatar");
    }
    else if ( type == QQMsg::kGroup )
    {
        path = main_win_->chatManager()->getGroupAvatarPath(uid);
        if ( path.isEmpty() )
            path = QQSkinEngine::instance()->getSkinRes("default_group_avatar");
    }

    return path;
}

void MsgTip::showMessage(ShareQQMsgPtr msg)
{
	SystemTray *trayIcon = SystemTray::instance();
	switch(msg->type())
	{
	case QQMsg::kSystem:
		trayIcon->showMessage(getItemAvatar(msg->talkTo(), msg->type()), "[" + convertor_->convert(msg->talkTo()) + "]" + "request to add you", msg->msg(),  NOTIFY_TIMEOUT_MS);
        break;

    case QQMsg::kSystemG:
		trayIcon->showMessage(getItemAvatar(msg->sendUin(), msg->type()), "[" + convertor_->convert(msg->sendUin()) + "]" + "request to enter group [" + convertor_->convert(msg->sendUin()), msg->msg(),  NOTIFY_TIMEOUT_MS);
        break;

    case QQMsg::kSess:
		trayIcon->showMessage(getItemAvatar(msg->sendUin(), msg->type()), "[" + convertor_->convert(msg->talkTo()) + "]" + " have new message", msg->msg(), NOTIFY_TIMEOUT_MS);
        break;

    case QQMsg::kFriend:
		trayIcon->showMessage(getItemAvatar(msg->sendUin(), msg->type()), "[" + convertor_->convert(msg->talkTo()) + "]" + " have new message", msg->msg(), NOTIFY_TIMEOUT_MS);
        break;

    case QQMsg::kGroup:
		trayIcon->showMessage(getItemAvatar(msg->talkTo(), msg->type()), "[" + convertor_->convert(msg->talkTo()) + "]" + " have new message", msg->msg(), NOTIFY_TIMEOUT_MS);
        break;
    }
}

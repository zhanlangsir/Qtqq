#include "msgtip.h"
#include "ui_msgtip.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QCursor>
#include <QMouseEvent>
#include <QModelIndex>
#include <QScrollBar>

#include "systemtray.h"
#include "mainwindow.h"
#include "frienditemmodel.h"
#include "groupitemmodel.h"

MsgTip::MsgTip(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MsgTip)
{
    ui->setupUi(this);
    setWindowOpacity(1);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Popup | Qt::FramelessWindowHint);
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
    lock.lock();
    for (int i = 0; i < ui->uncheckmsglist->count(); ++i)
    {
        ShareQQMsgPtr msg = ui->uncheckmsglist->item(i)->data(Qt::UserRole).value<ShareQQMsgPtr>();

        if (msg->talkTo() == new_msg->talkTo())
        {
            lock.unlock();
            return;
        }
    }
    addItem(new_msg);
    lock.unlock();
 }

void MsgTip::addItem(ShareQQMsgPtr msg)
{
    SystemTray *trayIcon = SystemTray::instance();

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
        trayIcon->showMessage("[" + convertor_->convert(msg->talkTo()) + "]" + "request to add you", msg->msg(),  200);
    }
        break;
    case QQMsg::kSystemG:
    {
        //QListWidgetItem *item = new QListWidgetItem("[" + convertor_->convert(msg->sendUin()) + "]" + "request to enter group [" + convertor_->convert(msg->sendUin()));
        QListWidgetItem *item = new QListWidgetItem("Group system message!");
        item->setData(Qt::UserRole, QVariant::fromValue(msg));
        ui->uncheckmsglist->addItem(item);

        trayIcon->showMessage("[" + convertor_->convert(msg->sendUin()) + "]" + "request to enter group [" + convertor_->convert(msg->sendUin()), msg->msg(),  200);
    }
        break;
    case QQMsg::kFriend:
    {
        //QListWidgetItem *item = new QListWidgetItem("[" + convertor_->convert(msg->talkTo()) + "]" + " send message to " + "[" + tr("you") + "]");
        QListWidgetItem *item = new QListWidgetItem(convertor_->convert(msg->talkTo()));
        item->setData(Qt::UserRole, QVariant::fromValue(msg));
        ui->uncheckmsglist->addItem(item);

        trayIcon->showMessage("[" + convertor_->convert(msg->talkTo()) + "]" + " send message to " + "[" + tr("you") + "]", msg->msg(), 200);
        break;
    }
    case QQMsg::kGroup:
    {
        //QListWidgetItem *item = new QListWidgetItem("[" + convertor_->convert(msg->talkTo()) + "]" + " send message to " + "[" + tr("you") + "]");
        QListWidgetItem *item = new QListWidgetItem(convertor_->convert(msg->talkTo()));
        item->setData(Qt::UserRole, QVariant::fromValue(msg));
        ui->uncheckmsglist->addItem(item);

        trayIcon->showMessage("[" + convertor_->convert(msg->talkTo()) + "]" + " send message to " + "[" + tr("you") + "]", msg->msg(), 200);
        break;
    }
    }

    int item_height = 25;
    this->resize(this->width(), ui->uncheckmsglist->count() * item_height);
    emit newUncheckMsgArrived();
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

void MsgTip::activatedChat(int i)
{
    if (ui->uncheckmsglist->count() < i+1)
        return;

    QListWidgetItem *item = ui->uncheckmsglist->item(i);
    ShareQQMsgPtr msg = item->data(Qt::UserRole).value<ShareQQMsgPtr>();
    removeItem(msg->talkTo());

    switch (msg->type())
    {
    case QQMsg::kFriend:
    case QQMsg::kGroup:
        emit activatedChatDlg(msg->type(), msg->talkTo(), msg->gCode());
        break;
    case QQMsg::kSystem:
        emit activateFriendRequestDlg(msg);
        break;
    case QQMsg::kSystemG:
        emit activateGroupRequestDlg(msg);
        break;
    }
}

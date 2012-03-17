#include "qqmsgtip.h"
#include "qqmsg.h"
#include <QDebug>
#include <QVBoxLayout>

void QQMsgTip::pushMsg(QQMsg* new_msg)
{
    if (new_msg->type() == QQMsg::kSystem)
    {
        addItem(new_msg);
        return;
    }

    if (new_msg->type() == QQMsg::kBuddiesStatusChange)
    {
        return;
    }

    lock.lock();
    for (int i = 0; i < box_.count(); ++i)
    {
        QQMsg* msg = box_.itemData(i).value<QQMsg*>();

        if (msg->talkTo() == new_msg->talkTo())
        {
            lock.unlock();
            return;
        }
    }
    addItem(new_msg);
    lock.unlock();
 }

void QQMsgTip::addItem(QQMsg* msg)
{
    switch(msg->type())
    {
    case QQMsg::kSystem:
        //box_.addItem(msg->tip(), QVariant::fromValue(msg));
        break;
    case QQMsg::kFriend:
    {
        QQChatMsg *chat_msg = static_cast<QQChatMsg*>(msg);
        box_.addItem("[" + convertor_->convert(chat_msg->from_uin_) + "]" + " send message to " + "[" + convertor_->convert(chat_msg->to_uin_) + "]", QVariant::fromValue(msg));
        break;
    }
    case QQMsg::kGroup:
    {
        QQChatMsg *g_chat_msg = static_cast<QQChatMsg*>(msg);
        box_.addItem("[" + convertor_->convert(g_chat_msg->to_uin_) + "]" + " send message to " + "[" + tr("you") + "]", QVariant::fromValue(msg));
        break;
    }
    }

    if (!this->isVisible())
    {
        emit addItemDone();
    }
}

void QQMsgTip::openChatDlg(int index)
{
    QQMsg *msg = box_.itemData(index).value<QQMsg*>();
    box_.removeItem(index);

    if (box_.count() == 0)
    {
        this->hide();
    }
    emit activatedChatDlg(msg->type(), msg->talkTo());
}

QQMsgTip::QQMsgTip(QWidget *parent) : QWidget(parent), box_(this)
{
    setWindowFlags(Qt::Dialog);
    this->setLayout(new QVBoxLayout(this));
    connect(&box_, SIGNAL(activated(int)), this, SLOT(openChatDlg(int)));
    connect(this, SIGNAL(addItemDone()), this, SLOT(show()));
}

#include "qqmsgtip.h"
#include "qqmsg.h"
#include <ui_qqmsgtip.h>
#include <QDebug>
#include <QVBoxLayout>
#include <QCursor>
#include <QMouseEvent>

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
    for (int i = 0; i < ui->cb_msgs_->count(); ++i)
    {
        QQMsg* msg = ui->cb_msgs_->itemData(i).value<QQMsg*>();

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
        ui->cb_msgs_->addItem("[" + convertor_->convert(chat_msg->from_uin_) + "]" + " send message to " + "[" + convertor_->convert(chat_msg->to_uin_) + "]", QVariant::fromValue(msg));
        break;
    }
    case QQMsg::kGroup:
    {
        QQChatMsg *g_chat_msg = static_cast<QQChatMsg*>(msg);
        ui->cb_msgs_->addItem("[" + convertor_->convert(g_chat_msg->to_uin_) + "]" + " send message to " + "[" + tr("you") + "]", QVariant::fromValue(msg));
        break;
    }
    }

    if (!this->isVisible())
    {
        emit addItemDone();
    }
}

void QQMsgTip::mousePressEvent(QMouseEvent *event)
{
  QPoint origin_pos = this->pos();

  QPoint origin_mouse_pos = QCursor::pos();
  distance_pos_ = origin_mouse_pos - origin_pos;
}

void QQMsgTip::mouseMoveEvent(QMouseEvent *event)
{
    if (distance_pos_.isNull())
    {
        return;
    }

    this->move(event->globalPos() - distance_pos_);
}

void QQMsgTip::enterEvent(QEvent *)
{
    setCursor(Qt::SizeAllCursor);
}

void QQMsgTip::leaveEvent(QEvent *)
{
    setCursor(Qt::ArrowCursor);
}

bool QQMsgTip::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::Enter)
    {
        setCursor(Qt::ArrowCursor);
        return true;
    }
    if (e->type() == QEvent::Leave)
    {
        setCursor(Qt::SizeAllCursor);
        return true;
    }
    else
        return QWidget::eventFilter(obj, e);
}

void QQMsgTip::mouseReleaseEvent(QMouseEvent *)
{
    distance_pos_ = QPoint(0, 0);
}

void QQMsgTip::openChatDlg(int index)
{
    QQMsg *msg = ui->cb_msgs_->itemData(index).value<QQMsg*>();
    ui->cb_msgs_->removeItem(index);

    if (ui->cb_msgs_->count() == 0)
    {
        this->hide();
    }
    emit activatedChatDlg(msg->type(), msg->talkTo());
}

QQMsgTip::QQMsgTip(QWidget *parent) : QWidget(parent), ui(new Ui::QQMsgTip)
{
    ui->setupUi(this);
    setWindowOpacity(1);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    ui->cb_msgs_->installEventFilter(this);
    connect(ui->cb_msgs_, SIGNAL(activated(int)), this, SLOT(openChatDlg(int)));
    connect(this, SIGNAL(addItemDone()), this, SLOT(show()));
}

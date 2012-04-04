#include "qqmsgtip.h"
#include "ui_qqmsgtip.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QCursor>
#include <QMouseEvent>

#include "qqmsg.h"

struct MsgTipItem
{
    QQMsg::MsgType type_;
    QString id_;
    QString code_;
};

Q_DECLARE_METATYPE(MsgTipItem)

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
        MsgTipItem item = ui->cb_msgs_->itemData(i).value<MsgTipItem>();

        if (item.id_ == new_msg->talkTo())
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
        MsgTipItem item = {msg->type(), msg->talkTo(), msg->gCode()};
        ui->cb_msgs_->addItem("[" + convertor_->convert(msg->talkTo()) + "]" + " send message to " + "[" + tr("you") + "]", QVariant::fromValue(item));
        break;
    }
    case QQMsg::kGroup:
    {
        MsgTipItem item = {msg->type(), msg->talkTo(), msg->gCode()};
        ui->cb_msgs_->addItem("[" + convertor_->convert(msg->talkTo()) + "]" + " send message to " + "[" + tr("you") + "]", QVariant::fromValue(item));
        break;
    }
    }

    if (!this->isVisible())
    {
        emit addItemDone();
    }
}

void QQMsgTip::removeItem(QString id)
{
    for (int i = 0; i < ui->cb_msgs_->count(); ++i)
    {
        MsgTipItem item = ui->cb_msgs_->itemData(i).value<MsgTipItem>();

        if (item.id_ == id)
        {
            ui->cb_msgs_->removeItem(i);
        }
    }

    if (ui->cb_msgs_->count() == 0)
        this->hide();
}

void QQMsgTip::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
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
    MsgTipItem item = ui->cb_msgs_->itemData(index).value<MsgTipItem>();
    ui->cb_msgs_->removeItem(index);

    if (ui->cb_msgs_->count() == 0)
    {
        this->hide();
    }
    emit activatedChatDlg(item.type_, item.id_, item.code_);
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

#include "qqmsgtip.h"
#include "ui_qqmsgtip.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QCursor>
#include <QMouseEvent>

QQMsgTip::QQMsgTip(QWidget *parent) : QWidget(parent), ui(new Ui::QQMsgTip)
{
    ui->setupUi(this);
    setWindowOpacity(1);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Dialog | Qt::FramelessWindowHint);
    qRegisterMetaType<SoundPlayer::SoundType>("SoundPlayer::SoundType");
    ui->cb_msgs_->installEventFilter(this);

    connect(ui->cb_msgs_, SIGNAL(activated(int)), this, SLOT(slotActivated(int)));
    connect(this, SIGNAL(addItemDone()), this, SLOT(show()));
    connect(this, SIGNAL(bibibi(SoundPlayer::SoundType)), this, SLOT(beginBibibi(SoundPlayer::SoundType)));
}

void QQMsgTip::pushMsg(ShareQQMsgPtr new_msg)
{
    if (new_msg->type() == QQMsg::kBuddiesStatusChange)
    {
        return;
    }

    if (new_msg->type() == QQMsg::kSystem || new_msg->type() == QQMsg::kSystemG)
    {
        emit bibibi(SoundPlayer::kSystem);
    }
    else
    {
        emit bibibi(SoundPlayer::kMsg);
    }
    lock.lock();
    for (int i = 0; i < ui->cb_msgs_->count(); ++i)
    {
        ShareQQMsgPtr msg = ui->cb_msgs_->itemData(i).value<ShareQQMsgPtr>();

        if (msg->talkTo() == new_msg->talkTo())
        {
            lock.unlock();
            return;
        }
    }
    addItem(new_msg);
    lock.unlock();
 }

void QQMsgTip::addItem(ShareQQMsgPtr msg)
{
    QQSystemTray *trayIcon = QQSystemTray::instance();
    switch(msg->type())
    {
    case QQMsg::kSystem:
        ui->cb_msgs_->addItem("[" + convertor_->convert(msg->talkTo()) + "]" + "request to add you", QVariant::fromValue(msg));
        trayIcon->showMessage("[" + convertor_->convert(msg->talkTo()) + "]" + "request to add you", msg->msg());
        break;
    case QQMsg::kSystemG:
        ui->cb_msgs_->addItem("[" + convertor_->convert(msg->sendUin()) + "]" + "request to enter group [" + convertor_->convert(msg->talkTo()), QVariant::fromValue(msg));
        trayIcon->showMessage("[" + convertor_->convert(msg->sendUin()) + "]" + "request to enter group [" + convertor_->convert(msg->talkTo()), msg->msg());
        break;
    case QQMsg::kFriend:
    {
        ui->cb_msgs_->addItem("[" + convertor_->convert(msg->talkTo()) + "]" + " send message to " + "[" + tr("you") + "]", QVariant::fromValue(msg));
        trayIcon->showMessage("[" + convertor_->convert(msg->talkTo()) + "]" + " send message to " + "[" + tr("you") + "]", msg->msg());
        break;
    }
    case QQMsg::kGroup:
    {
        ui->cb_msgs_->addItem("[" + convertor_->convert(msg->talkTo()) + "]" + " send message to " + "[" + tr("you") + "]", QVariant::fromValue(msg));
        trayIcon->showMessage("[" + convertor_->convert(msg->talkTo()) + "]" + " send message to " + "[" + tr("you") + "]", msg->msg());
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
        ShareQQMsgPtr msg = ui->cb_msgs_->itemData(i).value<ShareQQMsgPtr>();

        if (msg->talkTo() == id)
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

void QQMsgTip::beginBibibi(SoundPlayer::SoundType type)
{
    SoundPlayer::singleton()->play(type);
}

void QQMsgTip::mouseReleaseEvent(QMouseEvent *)
{
    distance_pos_ = QPoint(0, 0);
}

void QQMsgTip::slotActivated(int index)
{
    if (ui->cb_msgs_->count() < index+1)
        return;

    ShareQQMsgPtr msg = ui->cb_msgs_->itemData(index).value<ShareQQMsgPtr>();
    ui->cb_msgs_->removeItem(index);

    if (ui->cb_msgs_->count() == 0)
    {
        this->hide();
    }

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

#include "qqwidget.h"

#include <QMouseEvent>
#include <QDebug>
#include <QPalette>
#include <QVBoxLayout>
#include <QBitmap>
#include <QPainter>

#include "qqtitlebar.h"

QQWidget::QQWidget(QWidget *parent) :
    QWidget(parent),
    is_mouse_down_(false),
    left_(false),
    right_(false),
    bottom_(false)
{
    setMouseTracking(true);
    //setWindowFlags(Qt::FramelessWindowHint);

    QVBoxLayout *vlo_main = new QVBoxLayout();
    QQTitleBar *title_bar = new QQTitleBar(this);
    content_wid_ = new QWidget();
    vlo_main->addWidget(title_bar);
    vlo_main->addWidget(content_wid_);
    vlo_main->setContentsMargins(2, 0 , 2, 2);

    setLayout(vlo_main);

    content_wid_->setMouseTracking(true);
    content_wid_->installEventFilter(this);
}

void QQWidget::setBackground(QPixmap bg)
{
    pix_bg_ = bg;

    QPalette palette;
    palette.setBrush(this->backgroundRole(), QBrush(bg));
    this->setPalette(palette);
}

void QQWidget::mousePressEvent(QMouseEvent *e)
{
    old_pos_ = e->pos();
    is_mouse_down_ = e->button() == Qt::LeftButton;
}

void QQWidget::mouseMoveEvent(QMouseEvent *e)
{
    int x = e->x();
    int y = e->y();

    if (is_mouse_down_) {
        int dx = x - old_pos_.x();
        int dy = y - old_pos_.y();

        QRect g = geometry();

        if (left_)
            g.setLeft(g.left() + dx);
        if (right_)
            g.setRight(g.right() + dx);
        if (bottom_)
            g.setBottom(g.bottom() + dy);


        if (g.width() > this->minimumWidth() || g.height() > this->minimumHeight())
        {
            setGeometry(g);
        }

        old_pos_ = QPoint(!left_ ? e->x() : old_pos_.x(), e->y());

    } else {
        QRect r = rect();
        left_ = qAbs(x - r.left()) <= 2;
        right_ = qAbs(x - r.right()) <= 2;
        bottom_ = qAbs(y - r.bottom()) <= 2;

        bool hor = left_ | right_;

        if (hor && bottom_) {
            if (left_)
                setCursor(Qt::SizeBDiagCursor);
            else
                setCursor(Qt::SizeFDiagCursor);
        } else if (hor) {
            setCursor(Qt::SizeHorCursor);
        } else if (bottom_) {
            setCursor(Qt::SizeVerCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }
}

void QQWidget::mouseReleaseEvent(QMouseEvent *e)
{
    is_mouse_down_ = false;
}

bool QQWidget::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == content_wid_ && e->type() == QEvent::Enter)
    {
        setCursor(Qt::ArrowCursor);
        left_ = false;
        right_ = false;
        bottom_ = false;
    }

    return QWidget::eventFilter(obj, e);
}

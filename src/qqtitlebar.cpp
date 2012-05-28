#include "qqtitlebar.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QMouseEvent>

QQTitleBar::QQTitleBar(QWidget *parent) :
    QWidget(parent)
{
    setMouseTracking(true);
    QHBoxLayout *layout = new QHBoxLayout();
    setLayout(layout);

    lbl_title_ = new QLabel("Qtqq");
    lbl_title_->setObjectName("title");
    layout->addWidget(lbl_title_);

    QToolButton *tb_min = new QToolButton();
    tb_min->setIconSize(QSize(32,18));
    tb_min->setObjectName("tb_min");
    layout->addWidget(tb_min);

    QToolButton *tb_close = new QToolButton();
    tb_close->setObjectName("tb_close");
    tb_close->setIconSize(QSize(32,18));
    layout->addWidget(tb_close);


    layout->setSpacing(0);
    layout->setContentsMargins(5, 0, 0, 0);
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

    QMetaObject::connectSlotsByName(this);
}

void QQTitleBar::on_tb_min_clicked()
{
    parentWidget()->showMinimized();
}

void QQTitleBar::on_tb_close_clicked()
{
    parentWidget()->close();
}

void QQTitleBar::mousePressEvent(QMouseEvent *event)
{
    QPoint origin_pos = mapToGlobal(this->pos());
    QPoint origin_mouse_pos = QCursor::pos();

    distance_pos_ = origin_mouse_pos - origin_pos;
}

void QQTitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (distance_pos_.isNull())
    {
        return;
    }

    parentWidget()->move(event->globalPos() - distance_pos_);
}

void QQTitleBar::mouseReleaseEvent(QMouseEvent *)
{
    distance_pos_ = QPoint(0, 0);
}

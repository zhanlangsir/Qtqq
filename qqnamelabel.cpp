#include "qqnamelabel.h"

void QQNameLabel::mouseMoveEvent(QMouseEvent *ev)
{
    setCursor(Qt::PointingHandCursor);
}

QQNameLabel::QQNameLabel(QString text):QLabel(text)
{
}

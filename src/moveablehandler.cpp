#include "moveablehandler.h"

#include <QWidget>
#include <QEvent>
#include <QMouseEvent>

MoveableHandler::MoveableHandler(QWidget *parent) :
    QObject(parent),
    handing_widget_(parent)
{
    handing_widget_->installEventFilter(this);
}

bool MoveableHandler::eventFilter(QObject *obj, QEvent *e)
{
    switch (e->type())
    {
    case QEvent::MouseMove:
            handleMouseMoveEvent((QMouseEvent*)e);
            break;

    case QEvent::MouseButtonPress:
            handleMousePressEvent((QMouseEvent*)e);
            break;

    case QEvent::MouseButtonRelease:
            handleMouseReleaseEvent((QMouseEvent*)e);
            break;

    default:
        return handing_widget_->eventFilter(obj, e);
    }

    return handing_widget_->eventFilter(obj, e);;
}

void MoveableHandler::handleMouseMoveEvent(QMouseEvent *e)
{
    if (distance_pos_.isNull())
    {
        return;
    }

    handing_widget_->move(e->globalPos() - distance_pos_);
}

void MoveableHandler::handleMousePressEvent(QMouseEvent *e)
{
    QPoint origin_pos = handing_widget_->mapToGlobal(handing_widget_->pos());
    QPoint origin_mouse_pos = QCursor::pos();

    distance_pos_ = origin_mouse_pos - origin_pos;
}

void MoveableHandler::handleMouseReleaseEvent(QMouseEvent *e)
{
    distance_pos_ = QPoint(0, 0);
}

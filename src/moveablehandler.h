#ifndef QTQQ_CORE_MOVEABLEHANDLE_H
#define QTQQ_CORE_MOVEABLEHANDLE_H

#include <QObject>
#include <QPoint>

class QWidget;
class QEvent;
class QMouseEvent;

class  MoveableHandler : public QObject
{
public:
    explicit MoveableHandler(QWidget *parent = NULL);

protected:
    bool eventFilter(QObject *obj, QEvent *e);

private:
    void handleMouseMoveEvent(QMouseEvent *e);
    void handleMousePressEvent(QMouseEvent *e);
    void handleMouseReleaseEvent(QMouseEvent *e);

private:
    QWidget *handing_widget_;
    QPoint distance_pos_;
};

#endif  //QTQQ_CORE_MOVEABLEHANDLE_H

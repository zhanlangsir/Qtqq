#ifndef QTQQ_QQWINDOWWIDGET_H
#define QTQQ_QQWINDOWWIDGET_H

#include <QWidget>
#include <QPoint>
#include <QPixmap>

class QMouseEvent;
class QQTitleBar;

class QQWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QPixmap background READ getBackground WRITE setBackground)

public:
    explicit QQWidget(QWidget *parent = 0);

public:
    void setBackground(QPixmap pix);

    QPixmap getBackground() const
    { return pix_bg_; }

    QWidget *contentWidget() const
    { return content_wid_; }

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    bool eventFilter(QObject * obj, QEvent * e);

private:
    QPoint old_pos_;
    bool is_mouse_down_;
    bool left_,right_,bottom_;
    QWidget *content_wid_;

    QPixmap pix_bg_;
};

#endif // QTQQ_QQWINDOWWIDGET_H

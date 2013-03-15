#ifndef QQWIDGET_H
#define QQWIDGET_H

#include <QWidget>
#include <QPoint>
#include <QPixmap>

class QMouseEvent;
class QQTitleBar;
class QEvent;

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
    bool resize_, move_;
    bool left_,right_,bottom_;
    QWidget *content_wid_;

    QPixmap pix_bg_;
};

#endif //QQWIDGET_H

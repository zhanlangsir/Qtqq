#ifndef QTQQ_QQTITLEBAR_H
#define QTQQ_QQTITLEBAR_H

#include <QWidget>
#include <QPoint>
#include <QLabel>

class QQTitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit QQTitleBar(QWidget *parent = 0);
    void setTitle(QString title)
    {
        lbl_title_->setText(title);
    }

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

private slots:
    void on_tb_min_clicked();
    void on_tb_close_clicked();

private:
    QPoint distance_pos_; 
    QLabel *lbl_title_;
};

#endif // QTQQ_QQTITLEBAR_H

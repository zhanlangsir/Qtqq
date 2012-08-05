#ifndef QTQQ_QQFACEPANEL_H
#define QTQQ_QQFACEPANEL_H

#include <QDialog>

namespace Ui
{
    class QQFacePanel;
}

class QTableWidgetItem;

class QQFacePanel : public QDialog
{
    Q_OBJECT
public:
    QQFacePanel(QWidget *parent = NULL);
    ~QQFacePanel();

signals:
    void qqfaceClicked(QString id);

protected:
    void leaveEvent(QEvent *e);

private slots:
    void onItemClicked(QTableWidgetItem *item);

private:
    void loadFace();

private:
    Ui::QQFacePanel *ui_;
    QString qqface_path_;

    static const int kPerColumn_ = 15;
    static const int no2path[105];
};

#endif //QTQQ_QQFACEPANEL_H

#pragma once

namespace Ui
{
    class QQFacePanel;
}

#include <QWidget>
#include <QTableWidgetItem>

class QQFacePanel : public QWidget
{
    Q_OBJECT

public:
    QQFacePanel(QWidget *parent = NULL);

public:

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
    static const int kPerColumn_ = 14;
    QString qqface_path_;
};

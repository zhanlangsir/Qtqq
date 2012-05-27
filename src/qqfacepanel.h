#pragma once

namespace Ui
{
    class QQFacePanel;
}

#include <QDialog>
#include <QTableWidgetItem>

class QQFacePanel : public QDialog
{
    Q_OBJECT

public:
    QQFacePanel(QWidget *parent = NULL);
    ~QQFacePanel();

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

#ifndef QTQQ_QQFRIENDREQUESTDLG_H
#define QTQQ_QQFRIENDREQUESTDLG_H

namespace Ui
{
    class QQFriendRequestDlg;
}

#include <QDialog>

class QQMsg;
class FriendItemModel;

class QQFriendRequestDlg:public QDialog
{
    Q_OBJECT
public:
    //QQFriendRequestDlg(QWidget *parnet = 0);
    QQFriendRequestDlg(const QQMsg *msg, FriendItemModel *model, QWidget *parent = 0);

private slots:
    void slotOkClicked();
    void slotIgnoreClicked();
    void slotToggleDenyReason(bool check);

private:
    void initialize();

private:
    Ui::QQFriendRequestDlg *ui_;
    FriendItemModel *model_;
    QString account_;
    QString id_;
};


#endif // QTQQ_QQFRIENDREQUESTDLG_H

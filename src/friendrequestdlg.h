#ifndef QTQQ_FRIENDREQUESTDLG_H
#define QTQQ_FRIENDREQUESTDLG_H

namespace Ui
{
    class FriendRequestDlg;
}

#include <QDialog>

#include "core/qqmsg.h"

class FriendItemModel;

class FriendRequestDlg:public QDialog
{
    Q_OBJECT
public:
    //QQFriendRequestDlg(QWidget *parnet = 0);
    FriendRequestDlg(const ShareQQMsgPtr msg, FriendItemModel *model, QWidget *parent = 0);

private slots:
    void slotOkClicked();
    void slotIgnoreClicked();
    void slotToggleDenyReason(bool check);

private:
    void initialize();

private:
    Ui::FriendRequestDlg *ui_;
    FriendItemModel *model_;
    QString account_;
    QString id_;
};


#endif // QTQQ_FRIENDREQUESTDLG_H

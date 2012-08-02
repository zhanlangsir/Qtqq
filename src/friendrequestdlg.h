#ifndef QTQQ_FRIENDREQUESTDLG_H
#define QTQQ_FRIENDREQUESTDLG_H

#include <QDialog>

#include "core/qqmsg.h"

namespace Ui
{
    class FriendRequestDlg;
}


class FriendItemModel;

class FriendRequestDlg:public QDialog
{
    Q_OBJECT
public:
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

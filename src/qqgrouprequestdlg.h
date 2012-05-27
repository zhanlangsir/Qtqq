#ifndef QTQQ_GROUPREQUESTDLG_H
#define QTQQ_GROUPREQUESTDLG_H

namespace Ui
{
    class QQGroupRequestDlg;
}

#include <QDialog>

#include "core/qqmsg.h"

class FriendItemModel;
class GroupItemModel;

class QQGroupRequestDlg : public QDialog
{
    Q_OBJECT
public:
    QQGroupRequestDlg(const ShareQQMsgPtr msg, FriendItemModel *f_model, GroupItemModel *g_model, QWidget *parent = 0);

private slots:
    void slotOkClicked();
    void slotIgnoreClicked();
    void slotToggleDenyReason(bool check);

private:
    Ui::QQGroupRequestDlg *ui_;
    QString gid_;
    QString id_;
};

#endif // QTQQ_GROUPREQUESTDLG_H

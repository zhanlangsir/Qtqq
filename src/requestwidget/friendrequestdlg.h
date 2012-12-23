#ifndef FRIENDREQUESTDLG_H
#define FRIENDREQUESTDLG_H

#include <QDialog>
#include <QPixmap>
#include <QString>

#include "core/qqmsg.h"

namespace Ui
{
    class FriendRequestDlg;
}

class Contact;

class FriendItemModel;

class FriendRequestDlg:public QDialog
{
    Q_OBJECT
public:
    FriendRequestDlg(const ShareQQMsgPtr msg, Contact *contact, QWidget *parent = 0);

private slots:
    void slotOkClicked();
    void slotIgnoreClicked();
    void slotToggleDenyReason(bool check);

	void updateRequesterInfo(QString id, Contact *contact);
	void updateRequesterIcon(QString id, QPixmap pix);

private:
    void initUi(ShareQQMsgPtr msg);
    void initConnections();

private:
    Ui::FriendRequestDlg *ui_;
	Contact *contact_;
	QString id_;
	QString account_;
};

#endif //FRIENDREQUESTDLG_H

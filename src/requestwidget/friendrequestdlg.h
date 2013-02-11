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
    FriendRequestDlg(QString requester_id, QString requester_qq_number, QString msg, Contact *contact, QWidget *parent = NULL);

private slots:
    void slotOkClicked();
    void slotIgnoreClicked();
    void slotToggleDenyReason(bool check);

	void updateRequesterInfo(QString id, Contact *contact);
	void updateRequesterIcon(QString id, QPixmap pix);

private:
    void initUi(Contact *contact);
    void initConnections();
    ContactStatus extractStatus(const QByteArray &result);

private:
    Ui::FriendRequestDlg *ui_;
	QString id_;
	QString qq_number_;
    QString msg_;
};

#endif //FRIENDREQUESTDLG_H

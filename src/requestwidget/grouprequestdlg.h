#ifndef GROUPREQUESTDLG_H
#define GROUPREQUESTDLG_H

#include <QDialog>

#include "core/qqmsg.h"

namespace Ui
{
    class GroupRequestDlg;
}

class Contact;
class Group;

class GroupRequestDlg : public QDialog
{
    Q_OBJECT
public:
    GroupRequestDlg(const ShareQQMsgPtr msg, Contact *contact, Group *group, QWidget *parent = 0);

private slots:
    void slotOkClicked();
    void slotIgnoreClicked();
    void slotToggleDenyReason(bool check);

	void updateRequesterInfo(Contact *contact);
	void updateRequesterIcon(const QByteArray &icon_data);

private:
    Ui::GroupRequestDlg *ui_;

    QString gid_;
    QString id_;
};

#endif //GROUPREQUESTDLG_H

#ifndef QTQQ_LOGINDLG_H
#define QTQQ_LOGINDLG_H

#include <QWidget>

#include "core/accountmanager.h"

namespace Ui {
class LoginWin;
}

class LoginWin : public QWidget
{
    Q_OBJECT
signals:
    void sig_loginFinish();

public:
    explicit LoginWin(AccountManager &account_mgr, QWidget *parent = NULL);
    ~LoginWin();

    void login(QString uin, QString pwd, ContactStatus status, bool rem_pwd = false);

protected:
    bool eventFilter(QObject *obj, QEvent *e);
    void closeEvent(QCloseEvent *event);

private slots:
	void beginLogin();
    void onAutoLoginBtnClicked(bool checked);
    void currentUserChanged(QString text);
    void idChanged(QString text);
    void on_register_account_linkActivated(const QString &link);
    void on_find_password_linkActivated(const QString &link);

private:
    void setupStatus();
    void checkAccoutStatus(QString uin, QString pwd, ContactStatus status);
    QString showCapImg(QPixmap pix);
    ContactStatus getLoginStatus() const;
    void setupAccountRecords();
    void setUserLoginInfo(QString text);
    int getStatusIndex(ContactStatus status) const;
    
private:
    Ui::LoginWin *ui;

    AccountManager &account_manager_;
    AccountRecord curr_login_account_;
};

#endif //QTQQ_LOGINDLG_H

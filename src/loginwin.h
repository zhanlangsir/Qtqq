#ifndef QTQQ_LOGINDLG_H
#define QTQQ_LOGINDLG_H

#include "core/types.h"

#include <QDialog>
#include <QMouseEvent>
#include <QPoint>
#include <QVector>

#include "core/qqlogincore.h"

class QByteArray;
class QTcpSocket;

namespace Ui {
class LoginWin;
}

struct AccountRecord
{
    QString id_;
    QString pwd_;
    FriendStatus login_status_;
    bool rem_pwd_;
};

Q_DECLARE_METATYPE(AccountRecord*)

class LoginWin : public QWidget
{
    Q_OBJECT
signals:
    void sig_loginFinish();

public:
    explicit LoginWin(QQLoginCore *login_core, QWidget *parent = 0);
    ~LoginWin();

public:
    void saveConfig();

protected:
    bool eventFilter(QObject *obj, QEvent *e);

private slots:
    void onPbLoginClicked();
    void loginDone(QQLoginCore::LoginResult result);
    void onCekbAutoLoginClick(bool checked);
    void currentUserChanged(QString text);
    void idChanged(QString text);
    void on_register_account_linkActivated(const QString &link);
    void on_find_password_linkActivated(const QString &link);

private:
    void setupStatus();
    void checkAccoutStatus();
    void showCapImg(QPixmap pix);
    FriendStatus getLoginStatus() const;
    void readUsers();
    AccountRecord* findById(QString id) const;
    void setUserLoginInfo(QString text);
    int getStatusIndex(FriendStatus status) const;
    
private:

    Ui::LoginWin *ui;

    QQLoginCore *login_core_;
    QVector<AccountRecord*>login_records_;
    AccountRecord *curr_login_account_;
    QString auto_login_id_;
};

#endif //QTQQ_LOGINDLG_H

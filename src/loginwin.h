#ifndef QTQQ_LOGINDLG_H
#define QTQQ_LOGINDLG_H

#include "core/types.h"

#include <QDialog>
#include <QMouseEvent>
#include <QPoint>
#include <QVector>
#include <QApplication>

#include "core/qqlogincore.h"
#include "core/accountmanager.h"

class QByteArray;
class QTcpSocket;

namespace Ui {
class LoginWin;
}

class LoginWin : public QWidget
{
    Q_OBJECT
signals:
    void sig_loginFinish();

public:
    explicit LoginWin(QWidget *parent = NULL);
    ~LoginWin();

protected:
    bool eventFilter(QObject *obj, QEvent *e);
    void closeEvent(QCloseEvent *event)
    {
        Q_UNUSED(event)
        qApp->quit();
    }

private slots:
    void onLoginBtnClicked();
    void loginDone(QQLoginCore::LoginResult result);
    void onAutoLoginBtnClicked(bool checked);
    void currentUserChanged(QString text);
    void idChanged(QString text);
    void on_register_account_linkActivated(const QString &link);
    void on_find_password_linkActivated(const QString &link);

private:
    void setupStatus();
    void checkAccoutStatus();
    void showCapImg(QPixmap pix);
    FriendStatus getLoginStatus() const;
    void setupAccountRecords();
    void setUserLoginInfo(QString text);
    int getStatusIndex(FriendStatus status) const;
    
private:
    Ui::LoginWin *ui;

    AccountManager account_manager_;
    AccountRecord curr_login_account_;
    QQLoginCore *login_core_;
};

#endif //QTQQ_LOGINDLG_H

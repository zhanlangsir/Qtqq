#ifndef QTQQ_QQLOGINDLG_H
#define QTQQ_QQLOGINDLG_H

#include "core/types.h"

#include <QDialog>
#include <QMouseEvent>
#include <QPoint>
#include <QVector>

#include "core/qqlogincore.h"
#include "qqwidget.h"

class QByteArray;
class QTcpSocket;

namespace Ui {
class QQLoginWin;
}

struct LoginInfo
{
    QString id_;
    QString pwd_;
    FriendStatus login_status_;
    bool rem_pwd_;
};

Q_DECLARE_METATYPE(LoginInfo*)

class QQLoginWin : public QQWidget
{
    Q_OBJECT
signals:
    void sig_loginFinish();

public:
    explicit QQLoginWin(QQLoginCore *login_core, QWidget *parent = 0);
    ~QQLoginWin();

public:
    FriendInfo getCurrentUserInfo() const;
    void saveConfig();

protected:
    bool eventFilter(QObject *obj, QEvent *e);

private slots:
    void onPbLoginClicked();
    void loginDone(QQLoginCore::LoginResult result);
    void onCekbAutoLoginClick(bool checked);
    void currentUserChanged(QString text);
    void idChanged(QString text);

private:
    void setupStatus();
    void checkAccoutStatus();
    void showCapImg(QPixmap pix);
    FriendStatus getLoginStatus() const;
    void readUsers();
    LoginInfo* findById(QString id) const;
    void setUserLoginInfo(QString text);
    int getStatusIndex(FriendStatus status) const;
    
private:
    Ui::QQLoginWin *ui;

    QQLoginCore *login_core_;
    FriendInfo curr_user_info_;
    QVector<LoginInfo*>login_infos_;
    LoginInfo *curr_login_info_;
    QString auto_login_id_;
};

#endif //QTQQ_QQLOGINDLG_H

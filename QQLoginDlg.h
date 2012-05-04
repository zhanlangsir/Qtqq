#ifndef QTQQ_QQLOGINDLG_H
#define QTQQ_QQLOGINDLG_H

#include "types.h"

#include <QDialog>
#include <QMouseEvent>
#include <QPoint>
#include <QVector>

#include "QQLoginCore.h"

class QByteArray;
class QTcpSocket;

namespace Ui {
class QQLoginDlg;
}

struct LoginInfo
{
    QString id_;
    QString pwd_;
    FriendStatus login_status_;
    bool rem_pwd_;
};

Q_DECLARE_METATYPE(LoginInfo*)

class QQLoginDlg : public QDialog
{
    Q_OBJECT
signals:
    void sig_loginFinish();

public:
    explicit QQLoginDlg(QQLoginCore *login_core, QWidget *parent = 0);
    ~QQLoginDlg();

public:
    FriendInfo getCurrentUserInfo() const;
    void saveConfig();

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
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
    Ui::QQLoginDlg *ui;
    QPoint distance_pos_;

    QQLoginCore *login_core_;
    FriendInfo curr_user_info_;
    QVector<LoginInfo*>login_infos_;
    LoginInfo *new_login_info_;
    QString auto_login_id_;
};

#endif //QTQQ_QQLOGINDLG_H

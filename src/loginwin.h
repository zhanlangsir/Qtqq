#ifndef QTQQ_QQLOGINDLG_H
#define QTQQ_QQLOGINDLG_H

#include "core/types.h"

#include <QDialog>
#include <QMouseEvent>
#include <QPoint>
#include <QVector>

#include "core/qqlogincore.h"

class QByteArray;
class QTcpSocket;

namespace Ui {
class QQLoginWin;
}

struct AccountRecord
{
    QString id_;
    QString pwd_;
    FriendStatus login_status_;
    bool rem_pwd_;
};

Q_DECLARE_METATYPE(AccountRecord*)

class QQLoginWin : public QWidget
{
    Q_OBJECT
signals:
    void sig_loginFinish();

public:
    explicit QQLoginWin(QQLoginCore *login_core, QWidget *parent = 0);
    ~QQLoginWin();

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

    Ui::QQLoginWin *ui;

    QQLoginCore *login_core_;
    QVector<AccountRecord*>login_records_;
    AccountRecord *curr_login_account_;
    QString auto_login_id_;
};

#endif //QTQQ_QQLOGINDLG_H

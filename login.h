#pragma once

#include "types.h"

#include <QDialog>
#include <QMouseEvent>
#include <QPoint>

class QByteArray;
class QTcpSocket;

namespace Ui {
class QQLogin;
}

class QQLogin : public QDialog
{
    Q_OBJECT
    
public:
    explicit QQLogin(QWidget *parent = 0);
    ~QQLogin();

public:
    CaptchaInfo getCaptchaInfo() const;
    FriendInfo getCurrentUserInfo() const ;

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

private slots:
    void on_pb_login_clicked();
    void checkStateRead();
    void loginRead();
    void login();
    char getResultState(const QByteArray &array);
    void getLoginInfo(QString ptwebqq);
    void loginInfoRead();
    QByteArray getPwMd5();


private:
    void getCaptchaImg(QByteArray sum);
    
private:
    FriendInfo curr_user_info_;
    CaptchaInfo captcha_info_;
    Ui::QQLogin *ui;
    QString check_url_;
    QTcpSocket *fd_;
    QString vc_;
    QPoint distance_pos_;
};


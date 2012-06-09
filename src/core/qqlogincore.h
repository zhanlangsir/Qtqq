#ifndef QTQQ_QQLOGINCORE_H
#define QTQQ_QQLOGINCORE_H

#include <QTcpSocket>

#include "types.h"

class QByteArray;
class QTcpSocket;

#include <QObject>

class QQLoginCore : public QObject
{
    Q_OBJECT
public:
    enum AccountStatus {kNormal, kExceptionCpaImg};
    enum LoginResult {kIdOrPwdWrong, kAuthcodeWrong, kSucess, kUnknowErr};
    QQLoginCore();
    ~QQLoginCore();

public:
    QPixmap getCapImg();
    void login(QString id, QString pwd, FriendStatus status);
    void login(QString id, QString pwd, FriendStatus status, QString vc);
    AccountStatus checkState(QString id);

signals:
    void sig_loginDone(QQLoginCore::LoginResult result);

private slots:
    char getResultState(const QByteArray &array);
    void getLoginInfo(QString ptwebqq);
    void getLoginInfoDone();
    QByteArray hexchar2bin(const QByteArray &str);
    QByteArray getPwMd5(QString pwd);

private:
    QString getLoginStatus() const;
    void getCaptchaImg(QByteArray sum);
    void setupStatus(QString id, QString pwd, FriendStatus status);
    
private:
    QTcpSocket *fd_;
    QString vc_;
    QByteArray sum_;
    QByteArray uin_;

    QString id_;
    QString pwd_;
    FriendStatus status_;
};

#endif // QTQQ_QQLOGINCORE_H

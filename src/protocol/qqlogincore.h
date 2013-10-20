#ifndef QQLOGINCORE_H
#define QQLOGINCORE_H

#include <QByteArray>
#include <QTcpSocket>

#include "utils/contact_status.h"

class QTcpSocket;

class QQLoginCore
{
public:
    enum AccountStatus {kNormal, kExceptionCpaImg, kUnknowStatus};
    enum LoginResult {kIdOrPwdWrong, kAuthcodeWrong, kGetLoginInfoFailed, kSucess, kUnknowErr};
    QQLoginCore();
    ~QQLoginCore();

public:
    QPixmap getCapImg();
    LoginResult login(QString id, QString pwd, ContactStatus status);
    LoginResult login(QString id, QString pwd, ContactStatus status, QString vc);
    AccountStatus checkStatus(QString id);

private:
    char getResultState(const QByteArray &array);
    int getLoginInfo(QString ptwebqq);
    void getLoginInfoDone();
    QByteArray hexchar2bin(const QByteArray &str);
    QByteArray getPwMd5(QString pwd);

    QString getLoginStatus() const;
    void getCaptchaImg(QByteArray sum);
    void setupStatus(QString id, QString pwd, ContactStatus status);
	QByteArray getMd5Uin(const QByteArray &result, int begin_idx);
    QString getLoginSig();
    int getToken(QString get_token_url);
    
private:
    QTcpSocket *fd_;
    QString vc_;
    QByteArray sum_;
	QByteArray md5_uin_;
    QString login_sig_;

    QString id_;
    QString pwd_;
    ContactStatus status_;
};

#endif // QTQQ_QQLOGINCORE_H

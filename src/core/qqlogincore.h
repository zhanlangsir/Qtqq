#ifndef QQLOGINCORE_H
#define QQLOGINCORE_H

#include <QObject>
#include <QByteArray>
#include <QTcpSocket>

#include "utils/contact_status.h"

class QTcpSocket;

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
    void login(QString id, QString pwd, ContactStatus status);
    void login(QString id, QString pwd, ContactStatus status, QString vc);
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
    void setupStatus(QString id, QString pwd, ContactStatus status);
	QByteArray getMd5Uin(const QByteArray &result, int begin_idx);
    
private:
    QTcpSocket *fd_;
    QString vc_;
    QByteArray sum_;
	QByteArray md5_uin_;

    QString id_;
    QString pwd_;
    ContactStatus status_;
};

#endif // QTQQ_QQLOGINCORE_H

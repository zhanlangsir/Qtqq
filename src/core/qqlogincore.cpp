#include "qqlogincore.h"

#include <QByteArray>
#include <QDebug>
#include <QPixmap>
#include <QCryptographicHash>
#include <QTcpSocket>
#include <assert.h>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QSettings>
#include <QFile>

#include "sockethelper.h"
#include "captchainfo.h"
#include "request.h"

QQLoginCore::QQLoginCore()
{
    fd_ = new QTcpSocket();
}

QQLoginCore::~QQLoginCore()
{
    fd_->close();
    delete fd_;
}

void QQLoginCore::login(QString id, QString pwd, FriendStatus status)
{
    status_ = status;

    QString login_url = "/login?u=" + id + "&p=" + getPwMd5(pwd) + "&verifycode="+vc_+
            "&webqq_type=10&remember_uin=0&login2qq=1&aid=1003903&u1=http%3A%2F%2Fweb.qq.com%2Floginproxy.html%3Flogin2qq%3D1%26webqq_type%3D10&h=1&ptredirect=0&ptlang=2052&from_ui=1&pttype=1&dumy=&fp=loginerroralert&action=2-6-22950&mibao_css=m_webqq&t=1&g=1";

    qDebug()<<"login url"<<login_url<<endl;
    Request req;
    req.create(kGet, login_url);
    req.addHeaderItem("Host", "ptlogin2.qq.com");
    req.addHeaderItem("Cookie", CaptchaInfo::singleton()->cookie());
    req.addHeaderItem("Referer", "http://ui.ptlogin2.qq.com/cgi-bin/login?target=self&style=5&mibao_css=m_webqq&appid=1003903&enable_qlogin=0&no_verifyimg=1&s_url=http%3A%2F%2Fweb.qq.com%2Floginproxy.html&f_url=loginerroralert&strong_login=1&login_state=10&t=20120504001");

    fd_->connectToHost("ptlogin2.qq.com", 80);
    fd_->write(req.toByteArray());
    
    QByteArray result;
    while (result.indexOf(");") == -1 && fd_->waitForReadyRead(5000))
    {
        result.append(fd_->readAll());
    }

    fd_->close();
    qDebug()<<result<<endl;

    QString ptwebqq;

    char result_state = getResultState(result);
    qDebug()<<"login result state"<<result_state<<endl;

    switch (result_state)
    {
    case '0':
        break;
    case '3':
    {
        emit sig_loginDone(kIdOrPwdWrong);
        return;
    }
    case '4':
    {
        emit sig_loginDone(kAuthcodeWrong);
        return;
    }
    default:
        return;
    }

    int idx = 0;  
    
    while ((idx = result.indexOf("Set-Cookie:", idx)) != -1) 
    {
        idx += strlen("Set-Cookie: ");

        int value_idx = result.indexOf("=", idx); 
        int fin_value_idx = result.indexOf(";", idx);

        if (fin_value_idx == (value_idx + 1)) continue;

        QString key = result.mid(idx, value_idx - idx); 
        QString value = result.mid(value_idx+1, fin_value_idx - value_idx - 1);

        if (key == "ptwebqq")
            ptwebqq = value;

        if (key == "skey")
            CaptchaInfo::singleton()->set_skey(value);

        CaptchaInfo::singleton()->set_cookie(CaptchaInfo::singleton()->cookie() + key + "=" + value + ";");
    }

    getLoginInfo(ptwebqq);
}

void QQLoginCore::login(QString id, QString pwd, FriendStatus status, QString vc)
{
    vc_ = vc;
    login(id, pwd, status);
}

QQLoginCore::AccountStatus QQLoginCore::checkState(QString id)
{
    qDebug()<<"checking state"<<endl;
    curr_user_info_.set_id(id);
    QString check_url = "/check?uin=%1&appid=1003903&r=0.5354662109559408";
    fd_ = new QTcpSocket();
    fd_->connectToHost("check.ptlogin2.qq.com",80);

    Request req;
    req.create(kGet, check_url.arg(id));
    req.addHeaderItem("Host", "check.ptlogin2.qq.com");
    req.addHeaderItem("Connection", "Keep-Alive");

    fd_->write(req.toByteArray());
    fd_->waitForReadyRead();

    QByteArray result = fd_->readAll();
    qDebug()<<"check status result"<<result<<endl;
    fd_->disconnectFromHost();
    if (result.contains('!'))
    {
        int vc_idx = result.indexOf('!');
        vc_ = result.mid(vc_idx, 4);

        int uin_s_idx = result.indexOf('\'', vc_idx + 5) + 1;
        int uin_e_idx = result.indexOf('\'', uin_s_idx);

        QString uin = result.mid(uin_s_idx, uin_e_idx - uin_s_idx);
        QStringList dex_uin = uin.split("\\x");

        QByteArray dec_uin;
        QString dex;

        foreach (dex, dex_uin)
        {
            if (dex.isEmpty())
                continue;

            bool ok;
            dec_uin.append((char)dex.toInt(&ok, 16));
        }

        uin_ = dec_uin;

        int cookie_idx = result.indexOf("ptvfsession");
        int idx = result.indexOf(';', cookie_idx)+1;
        qDebug()<<"cookie"<<result.mid(cookie_idx, idx - cookie_idx)<<endl;
        CaptchaInfo::singleton()->set_cookie(result.mid(cookie_idx, idx - cookie_idx));

        return kNormal;
    }
    else
    {
        int ptui_idx = result.indexOf("ptui");
        int first_idx = result.indexOf(',', ptui_idx)+2;
        int second_idx = result.indexOf('\'', first_idx);

        sum_ = result.mid(first_idx, second_idx - first_idx);

        return kExceptionCpaImg;
    }
}

QPixmap QQLoginCore::getCapImg()
{
    QString captcha_str ="/getimage?uin=%1&vc_type=%2&aid=1003909&r=0.5354663109529408";

    Request req;
    req.create(kGet, captcha_str.arg(curr_user_info_.id()).arg(QString(sum_)));
    req.addHeaderItem("Host", "captcha.qq.com");
    req.addHeaderItem("Connection", "Keep-Alive");

    fd_->connectToHost("captcha.qq.com", 80);

    fd_->write(req.toByteArray());

    QByteArray result;
    socketReceive(fd_, result);
    
    int cookie_idx = result.indexOf("Set-Cookie") + 12;
    int idx = result.indexOf(';', cookie_idx)+1;
    CaptchaInfo::singleton()->set_cookie(result.mid(cookie_idx, idx - cookie_idx));

    QPixmap pix;
    pix.loadFromData(result.mid(result.indexOf("\r\n\r\n") + 4));

    fd_->close();

    return pix;
}

QString QQLoginCore::getLoginStatus() const
{
    switch (status_)
    {
    case kOnline:
        return "online";
    case kCallMe:
        return "callme";
    case kAway:
        return "away";
    case kBusy:
        return "busy";
    case kSilent:
        return "silent";
    case kHidden:
        return "hidden";
    default:
        break;
    }
    return "online";
}

void QQLoginCore::getLoginInfo(QString ptwebqq)
{
    qDebug()<<"getting login info"<<endl;
    QString login_info_path = "/channel/login2";
    QByteArray msg = "r={\"status\":\""+ getLoginStatus().toAscii() +"\",\"ptwebqq\":\"" + ptwebqq.toAscii() + "\","
            "\"passwd_sig\":""\"\",\"clientid\":\"5412354841\""
            ",\"psessionid\":null}&clientid=12354654&psessionid=null";

    Request req;
    req.create(kPost, login_info_path);
    req.addHeaderItem("Host", "d.web2.qq.com");
    req.addHeaderItem("Cookie", CaptchaInfo::singleton()->cookie());
    req.addHeaderItem("Referer", "http://d.web2.qq.com/channel/login2");
    req.addHeaderItem("Content-Length", QString::number(msg.length()));
    req.addHeaderItem("Content-Type", "application/x-www-form-urlencoded");
    req.addRequestContent(msg);

    fd_->connectToHost("d.web2.qq.com", 80);
    connect(fd_, SIGNAL(readyRead()), this, SLOT(getLoginInfoDone()));
    fd_->write(req.toByteArray());
}

void QQLoginCore::getLoginInfoDone()
{
    QByteArray result = fd_->readAll();
    qDebug()<<"get login info result"<<result<<endl;
    int vfwebqq_f_idx = result.indexOf("vfwebqq") + 10;
    int vfwebqq_s_idx = result.indexOf(',', vfwebqq_f_idx) - 1;

    CaptchaInfo::singleton()->set_vfwebqq(result.mid(vfwebqq_f_idx, vfwebqq_s_idx - vfwebqq_f_idx));

    int psessionid_f_idx = result.indexOf("psessionid") + 13;
    int  psessionid_s_idx = result.indexOf(',',  psessionid_f_idx) - 1;
    CaptchaInfo::singleton()->set_psessionid(result.mid( psessionid_f_idx,  psessionid_s_idx -  psessionid_f_idx));

    emit sig_loginDone(kSucess);
}

QByteArray QQLoginCore::hexchar2bin(const QByteArray &str)
{
    QByteArray result;
    for ( int i = 0; i < str.length(); i += 2 )
    {
        bool ok;
        result += QString::number(str.mid(i, i + 2).toInt(&ok, 16)).toAscii();
    }
    return result;
}

QByteArray QQLoginCore::getPwMd5(QString pwd)
{
    QByteArray md5;   
    md5 = QCryptographicHash::hash(pwd.toAscii(), QCryptographicHash::Md5);
    md5 = QCryptographicHash::hash(md5.append(uin_), QCryptographicHash::Md5).toHex().toUpper();
    md5 = QCryptographicHash::hash(md5.append(vc_.toUpper()), QCryptographicHash::Md5).toHex().toUpper();

    return md5;
}

char QQLoginCore::getResultState(const QByteArray &array)
{
    int idx = array.indexOf("ptuiCB");
    return array[idx + 8];
}


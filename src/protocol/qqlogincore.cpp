#include "qqlogincore.h"

#include <QByteArray>
#include <QDebug>
#include <QPixmap>
#include <QByteArray>
#include <QCryptographicHash>
#include <QTcpSocket>
#include <assert.h>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QSettings>
#include <QFile>

#include "json/json.h"

#include "core/sockethelper.h"
#include "core/captchainfo.h"
#include "core/request.h"
#include "core/curr_login_account.h"

QQLoginCore::QQLoginCore()
{
    fd_ = new QTcpSocket();
}

QQLoginCore::~QQLoginCore()
{
    fd_->close();
    fd_->deleteLater();
    delete fd_;
}

QString QQLoginCore::getLoginSig()
{
    QString get_sig_url = "/cgi-bin/login?daid=164&target=self&style=5&mibao_css=m_webqq&appid=1003903&enable_qlogin=0&s_url=http%3A%2F%2Fweb2.qq.com%2Floginproxy.html";

    Request req;
    req.create(kGet, get_sig_url);
    req.addHeaderItem("Host", "ui.ptlogin2.qq.com");
    req.addHeaderItem("Cookie", CaptchaInfo::instance()->cookie());

    fd_->connectToHost("ui.ptlogin2.qq.com", 80);
    fd_->write(req.toByteArray());
    
    QByteArray result;
    socketReceive(fd_, result);

    if ( result.isEmpty() )
    {
        return QString();
    }

    QByteArray result_body;
    QQUtility::gzdecompress(result.mid(result.indexOf("\r\n\r\n")+4), result_body);
    //qDebug() << "get login sig result: " << result_body << endl;

    if ( result_body.isEmpty() )
    {
        return QString();
    }

    QRegExp sig_reg("var g_login_sig=encodeURIComponent\\(\"(.*)\"\\)");
    sig_reg.setMinimal(true);
    if ( sig_reg.indexIn(result_body) == -1 )
    {
        return QString();
    }

    return sig_reg.cap(1);
}

QQLoginCore::LoginResult QQLoginCore::login(QString id, QString pwd, ContactStatus status)
{
	id_ = id;
	pwd_ = pwd;
    status_ = status;

    QString login_url = "/login?u="+ id + "&p=" + getPwMd5(pwd) + "&verifycode=" + vc_ + "&webqq_type=10&remember_uin=1&login2qq=1&aid=1003903&u1=http%3A%2F%2Fweb2.qq.com%2Floginproxy.html%3Flogin2qq%3D1%26webqq_type%3D10&h=1&ptredirect=0&ptlang=2052&daid=164&from_ui=1&pttype=1&dumy=&fp=loginerroralert&action=9-27-9657582&mibao_css=m_webqq&t=2&g=1&js_type=0&js_ver=10049&login_sig="+login_sig_;

    Request req;
    req.create(kGet, login_url);
    req.addHeaderItem("Host", "ptlogin2.qq.com");
    req.addHeaderItem("Cookie", CaptchaInfo::instance()->cookie());
    req.addHeaderItem("Referer", "http://ui.ptlogin2.qq.com/cgi-bin/login?daid=164&target=self&style=5&mibao_css=m_webqq&appid=1003903&enable_qlogin=0&no_verifyimg=1&s_url=http%3A%2F%2Fweb2.qq.com%2Floginproxy.html&f_url=loginerroralert&strong_login=1&login_state=10&t=20130903001");

    fd_->disconnectFromHost();
    fd_->connectToHost("ptlogin2.qq.com", 80);
    fd_->write(req.toByteArray());
    
    qDebug() << "login request: \n" << req.toByteArray() << endl;
    
    QByteArray result;
    while (result.indexOf(");") == -1 && fd_->waitForReadyRead(5000))
    {
        result.append(fd_->readAll());
    }

    fd_->close();

    qDebug() << "Login Result:\n" << result << '\n' << endl;
    if ( result.isEmpty() )
    {
        return kUnknowErr;
    }

    QString ptwebqq;

    char result_state = getResultState(result);

    switch (result_state)
    {
    case '0':
        break;
    case '3':
    {
        return kIdOrPwdWrong;
    }
    case '4':
    {
        return kAuthcodeWrong;
    }
    default:
        return kUnknowErr;
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
        {
            ptwebqq = value;
            CaptchaInfo::instance()->setPtwebqq(ptwebqq);
        }

        if (key == "skey")
            CaptchaInfo::instance()->setSkey(value);

        CaptchaInfo::instance()->setCookie(CaptchaInfo::instance()->cookie() + key + "=" + value + ";");
    }

    int url_begin_idx = result.indexOf("http");
    int url_end_idx = result.indexOf("'", url_begin_idx);
    QString get_token_url = result.mid(url_begin_idx, url_end_idx - url_begin_idx);
    qDebug() << "get_token_url: " << get_token_url << endl;

    getToken(get_token_url);

    if ( getLoginInfo(ptwebqq) != 0 )
    {
        return kGetLoginInfoFailed;
    }

    CurrLoginAccount::setPwd(pwd);
    return kSucess;
}

int QQLoginCore::getToken(QString get_token_url)
{
    int host_start_idx = get_token_url.indexOf("http://")+7;
    int host_end_idx = get_token_url.indexOf("com/", host_start_idx)+3;
    QString host = get_token_url.mid(host_start_idx, host_end_idx - host_start_idx);
    QString get_token_query_path = get_token_url.mid(host_end_idx);
    qDebug() << "get token host: " << host << "query path:" << get_token_query_path << endl;

    Request req;
    req.create(kGet, get_token_query_path);
    req.addHeaderItem("Host", host);
    req.addHeaderItem("Cookie", CaptchaInfo::instance()->cookie());

    qDebug() << "get token request: \n" << req.toByteArray() << endl;
    fd_->disconnectFromHost();
    fd_->connectToHost(host, 80);
    fd_->write(req.toByteArray());
    
    QByteArray result;
    socketReceive(fd_, result);

    if ( result.isEmpty() )
    {
        return -1;
    }
    qDebug() << "getToken result: \n" << result << endl;

    int idx = 0;
    while ((idx = result.indexOf("Set-Cookie:", idx)) != -1) 
    {
        idx += strlen("Set-Cookie: ");

        int value_idx = result.indexOf("=", idx); 
        int fin_value_idx = result.indexOf(";", idx);

        if (fin_value_idx == (value_idx + 1)) continue;

        QString key = result.mid(idx, value_idx - idx); 
        QString value = result.mid(value_idx+1, fin_value_idx - value_idx - 1);

        if ( (key == "p_skey" || key == "pt4_token" || key == "p_uin" ) && !value.isEmpty() )
        {
            qDebug() << key << value << endl;
            CaptchaInfo::instance()->setCookie(CaptchaInfo::instance()->cookie() + key + "=" + value + ";");
        }
    }
    return 0;
}

QQLoginCore::LoginResult QQLoginCore::login(QString id, QString pwd, ContactStatus status, QString vc)
{
    vc_ = vc;
    return login(id, pwd, status);
}

QByteArray QQLoginCore::getMd5Uin(const QByteArray &result, int begin_idx)
{
	int uin_s_idx = result.indexOf('\'', begin_idx) + 1;
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
	return dec_uin;
}

QQLoginCore::AccountStatus QQLoginCore::checkStatus(QString id)
{
    qDebug()<<"Checking Account Status"<<endl;
    if ( login_sig_.isEmpty() )
    {
        login_sig_ = getLoginSig();
        if ( login_sig_.isEmpty() )
        {
            return kUnknowStatus;
        }
        qDebug() << "QQLoginCore::checkStatus! get loginsig: " << login_sig_ << endl;
    }

	CurrLoginAccount::setId(id);
    QString check_url = "/check?uin=%1&appid=1003903&js_ver=10049&js_type=0&login_sig=%2&u1=http%3A%2F%2Fweb2.qq.com%2Floginproxy.html&r=0.8177125074315685";
    fd_->disconnectFromHost();
    fd_->connectToHost("check.ptlogin2.qq.com",80);

    Request req;
    req.create(kGet, check_url.arg(id).arg(login_sig_));
    req.addHeaderItem("Host", "check.ptlogin2.qq.com");
    req.addHeaderItem("Connection", "Keep-Alive");

    fd_->write(req.toByteArray());
    fd_->waitForReadyRead();

    QByteArray result = fd_->readAll();
    qDebug() << "Acount status:\n" << result << '\n' << endl;
    fd_->disconnectFromHost();

    if (result.contains('!'))
    {
        int vc_idx = result.indexOf('!');
        vc_ = result.mid(vc_idx, 4);

		md5_uin_ = getMd5Uin(result, vc_idx+5);

        int cookie_idx = result.indexOf("ptvfsession");
        int idx = result.indexOf(';', cookie_idx)+1;
        QString ptvfsession = result.mid(cookie_idx, idx - cookie_idx);
        qDebug() << "Extract ptvfseesion: " << ptvfsession << endl;
        CaptchaInfo::instance()->setCookie(ptvfsession);

        return kNormal;
    }
    else
    {
        int ptui_idx = result.indexOf("ptui");
		int sum_s_idx = result.indexOf(',', ptui_idx)+2;
		int sum_e_idx = result.indexOf('\'', sum_s_idx);

		sum_ = result.mid(sum_s_idx, sum_e_idx - sum_s_idx);

		md5_uin_ = getMd5Uin(result, sum_e_idx+2);

        return kExceptionCpaImg;
    }
}

QPixmap QQLoginCore::getCapImg()
{
    QString captcha_str ="/getimage?uin=%1&vc_type=%2&aid=1003909&r=0.5354663109529408";

    Request req;
    req.create(kGet, captcha_str.arg(CurrLoginAccount::id()).arg(QString(sum_)));
    req.addHeaderItem("Host", "captcha.qq.com");
    req.addHeaderItem("Connection", "Keep-Alive");

    fd_->connectToHost("captcha.qq.com", 80);

    fd_->write(req.toByteArray());

    QByteArray result;
    socketReceive(fd_, result);
    
    int cookie_idx = result.indexOf("Set-Cookie") + 12;
    int idx = result.indexOf(';', cookie_idx)+1;
    CaptchaInfo::instance()->setCookie(result.mid(cookie_idx, idx - cookie_idx));

    QPixmap pix;
    pix.loadFromData(result.mid(result.indexOf("\r\n\r\n") + 4));

    fd_->close();

    return pix;
}

QString QQLoginCore::getLoginStatus() const
{
    switch (status_)
    {
    case CS_Online:
        return "online";
    case CS_CallMe:
        return "callme";
    case CS_Away:
        return "away";
    case CS_Busy:
        return "busy";
    case CS_Silent:
        return "silent";
    case CS_Hidden:
        return "hidden";
    default:
        break;
    }
    return "online";
}

int QQLoginCore::getLoginInfo(QString ptwebqq)
{
    QString login_info_path = "/channel/login2";
    QByteArray msg = "r={\"status\":\""+ getLoginStatus().toLatin1() +"\",\"ptwebqq\":\"" + ptwebqq.toLatin1() + "\","
            "\"passwd_sig\":""\"\",\"clientid\":\"5412354841\""
            ",\"psessionid\":null}&clientid=12354654&psessionid=null";

    Request req;
    req.create(kPost, login_info_path);
    req.addHeaderItem("Host", "d.web2.qq.com");
    req.addHeaderItem("Cookie", CaptchaInfo::instance()->cookie());
    req.addHeaderItem("Referer", "http://d.web2.qq.com/channel/login2");
    req.addHeaderItem("Content-Length", QString::number(msg.length()));
    req.addHeaderItem("Content-Type", "application/x-www-form-urlencoded");
    req.addRequestContent(msg);

    fd_->disconnectFromHost();
    fd_->connectToHost("d.web2.qq.com", 80);
    fd_->write(req.toByteArray());

    QByteArray result;
    while (fd_->waitForReadyRead(2000))
    {
        result.append(fd_->readAll());
    }

    qDebug() <<"Got login information:\n" << result << '\n' << endl;

    QByteArray body = result.mid(result.indexOf("\r\n\r\n")+4);


    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(QString(body).toStdString(), root, false))
    {
        qDebug() << "parse failed!" << endl;
        return -1;
    }    

    Json::Value json_result = root["result"];
    QString vfwebqq = QString::fromStdString(json_result["vfwebqq"].asString());
    QString psessionid = QString::fromStdString(json_result["psessionid"].asString());
    int index = json_result["index"].asInt();
    int port = json_result["port"].asInt();
    /*
    int vfwebqq_f_idx = result.indexOf("vfwebqq") + 10;
    int vfwebqq_s_idx = result.indexOf(',', vfwebqq_f_idx) - 1;


    int psessionid_f_idx = result.indexOf("psessionid") + 13;
    int  psessionid_s_idx = result.indexOf(',',  psessionid_f_idx) - 1;
    */
    CaptchaInfo::instance()->setVfwebqq(vfwebqq);
    CaptchaInfo::instance()->setPsessionid(psessionid);
    CaptchaInfo::instance()->setIndex(index);
    CaptchaInfo::instance()->setPort(port);

    CurrLoginAccount::setId(id_);
    CurrLoginAccount::setStatus(status_);

    return 0;
}

void QQLoginCore::getLoginInfoDone()
{
    QByteArray result = fd_->readAll();
    qDebug() <<"Got login information:\n" << result << '\n' << endl;

    QByteArray body = result.mid(result.indexOf("\r\n\r\n")+4);


    Json::Reader reader;
    Json::Value root;

    if (!reader.parse(QString(body).toStdString(), root, false))
    {
//        emit sig_loginDone(kUnknowErr);
        return;
    }    

    Json::Value json_result = root["result"];
    QString vfwebqq = QString::fromStdString(json_result["vfwebqq"].asString());
    QString psessionid = QString::fromStdString(json_result["psessionid"].asString());
    int index = json_result["index"].asInt();
    int port = json_result["port"].asInt();
    /*
    int vfwebqq_f_idx = result.indexOf("vfwebqq") + 10;
    int vfwebqq_s_idx = result.indexOf(',', vfwebqq_f_idx) - 1;


    int psessionid_f_idx = result.indexOf("psessionid") + 13;
    int  psessionid_s_idx = result.indexOf(',',  psessionid_f_idx) - 1;
    */
    CaptchaInfo::instance()->setVfwebqq(vfwebqq);
    CaptchaInfo::instance()->setPsessionid(psessionid);
    CaptchaInfo::instance()->setIndex(index);
    CaptchaInfo::instance()->setPort(port);

    CurrLoginAccount::setId(id_);
    CurrLoginAccount::setStatus(status_);

    //emit sig_loginDone(kSucess);
}

QByteArray QQLoginCore::hexchar2bin(const QByteArray &str)
{
    QByteArray result;
    for ( int i = 0; i < str.length(); i += 2 )
    {
        bool ok;
        result += QString::number(str.mid(i, i + 2).toInt(&ok, 16)).toLatin1();
    }
    return result;
}

QByteArray QQLoginCore::getPwMd5(QString pwd)
{
    QByteArray md5;   
    md5 = QCryptographicHash::hash(pwd.toLatin1(), QCryptographicHash::Md5);
	md5 = QCryptographicHash::hash(md5.append(md5_uin_), QCryptographicHash::Md5).toHex().toUpper();
    md5 = QCryptographicHash::hash(md5.append(vc_.toUpper()), QCryptographicHash::Md5).toHex().toUpper();

    return md5;
}

char QQLoginCore::getResultState(const QByteArray &array)
{
    int idx = array.indexOf("ptuiCB");
	return array[idx + 8];
}

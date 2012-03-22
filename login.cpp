#include "login.h"
#include "ui_qqlogin.h"
#include "ui_captcha.h"
#include "types.h"
#include "request.h"

#include <QByteArray>
#include <QDebug>
#include <QPixmap>
#include <QCryptographicHash>
#include <QTcpSocket>
#include <assert.h>
#include <QMessageBox>
#include <QDesktopWidget>

QQLogin::QQLogin(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QQLogin)
{
    ui->setupUi(this);
    connect(ui->pb_login_, SIGNAL(clicked()), this, SLOT(on_pb_login_clicked()));
    connect(ui->tb_mini_, SIGNAL(clicked()), this, SLOT(showMinimized()));
    connect(ui->tb_close_, SIGNAL(clicked()), this, SLOT(reject()));
    setWindowOpacity(1);
    setWindowFlags(Qt::FramelessWindowHint);


    move((QApplication::desktop()->width() - this->width()) /2, (QApplication::desktop()->height() - this->height()) /2);
}

QQLogin::~QQLogin()
{
    fd_->close();
    delete ui;
}

void QQLogin::on_pb_login_clicked()
{
    if (ui->le_username_->text().isEmpty() || ui->le_password_->text().isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setInformativeText(tr("username or password is empty! please input them"));
        box.exec();
        return;
    }

    curr_user_info_.set_id(ui->le_username_->text());
    QString check_url = "/check?uin=%1&appid=1003909&r=0.5354662109559408";
    fd_ = new QTcpSocket();
    fd_->connectToHost("ptlogin2.qq.com",80);

    Request req;
    req.create(kGet, check_url.arg(curr_user_info_.id()));
    req.addHeaderItem("Host", "ptlogin2.qq.com");
    req.addHeaderItem("Connection", "Keep-Alive");

    connect(fd_, SIGNAL(readyRead()), this, SLOT(checkStateRead()));
    fd_->write(req.toByteArray());
}

void QQLogin::checkStateRead()
{
    fd_->disconnectFromHost();
    QByteArray result = fd_->readAll();

    if (result.contains('!'))
    {
        vc_ = result.indexOf('!');
    }
    else
    {
        int second_idx = result.lastIndexOf('\'');
        int first_idx = result.lastIndexOf('\'', second_idx-1)+1;

        getCaptchaImg(result.mid(first_idx, second_idx-first_idx));
    }

    disconnect(fd_,SIGNAL(readyRead()), this, SLOT(checkStateRead()));
}

void QQLogin::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
  QPoint origin_pos = this->pos();

  QPoint origin_mouse_pos = QCursor::pos();
  distance_pos_ = origin_mouse_pos - origin_pos;
}

void QQLogin::mouseMoveEvent(QMouseEvent *event)
{
    if (distance_pos_.isNull())
    {
        return;
    }

    this->move(event->globalPos() - distance_pos_);
}

void QQLogin::mouseReleaseEvent(QMouseEvent *)
{
    distance_pos_ = QPoint(0, 0);
}

CaptchaInfo QQLogin::getCaptchaInfo() const
{
    return captcha_info_;
}

FriendInfo QQLogin::getCurrentUserInfo() const
{
    return curr_user_info_;
}

void QQLogin::getCaptchaImg(QByteArray sum)
{
    QString captcha_str ="/getimage?uin=%1&vc_type=%2&aid=1003909&r=0.5354663109529408";

    Request req;
    req.create(kGet, captcha_str.arg(curr_user_info_.id()).arg(QString(sum)));
    req.addHeaderItem("Host", "captcha.qq.com");
    req.addHeaderItem("Connection", "Keep-Alive");

    fd_->connectToHost("captcha.qq.com", 80);

    fd_->write(req.toByteArray());

    QByteArray result;
    while (fd_->waitForReadyRead())
    {
        result.append(fd_->readAll());
    }

    int cookie_idx = result.indexOf("Set-Cookie") + 12;
    int idx = result.indexOf(';', cookie_idx)+1;
    captcha_info_.cookie_ = result.mid(cookie_idx, idx - cookie_idx);

    QDialog *captcha_dialog = new QDialog(this);
    Ui::QQCaptcha *ui = new Ui::QQCaptcha;
    QPixmap *pix = new QPixmap;
    pix->loadFromData(result.mid(result.indexOf("\r\n\r\n") + 4));
    ui->setupUi(captcha_dialog);
    ui->lbl_captcha_->setPixmap(*pix);

    if (captcha_dialog->exec())
    {
        vc_ = ui->le_captcha_->text().toUpper();
    }

    delete captcha_dialog;

    fd_->disconnectFromHost();

    login();
}

void QQLogin::getLoginInfo(QString ptwebqq)
{
    QString login_info_path = "/channel/login2";
    QByteArray msg = "r={\"status\":\"online\",\"ptwebqq\":\"" + ptwebqq.toAscii() + "\","
            "\"passwd_sig\":""\"\",\"clientid\":\"5412354841\""
            ",\"psessionid\":null}&clientid=12354654&psessionid=null";

    Request req;
    req.create(kPost, login_info_path);
    req.addHeaderItem("Host", "d.web2.qq.com");
    req.addHeaderItem("Cookie", captcha_info_.cookie_);
    req.addHeaderItem("Referer", "http://d.web2.qq.com/channel/login2");
    req.addHeaderItem("Content-Length", QString::number(msg.length()));
    req.addHeaderItem("Content-Type", "application/x-www-form-urlencoded");
    req.addRequestContent(msg);

    fd_->connectToHost("d.web2.qq.com", 80);
    connect(fd_, SIGNAL(readyRead()), this, SLOT(loginInfoRead()));
    fd_->write(req.toByteArray());
}

QByteArray QQLogin::getPwMd5()
{
    QByteArray md5;
    md5 = QCryptographicHash::hash(QByteArray(ui->le_password_->text().toAscii()), QCryptographicHash::Md5);
    md5 = QCryptographicHash::hash(md5, QCryptographicHash::Md5);
    md5 = QCryptographicHash::hash(md5, QCryptographicHash::Md5).toHex();
    md5 = QCryptographicHash::hash(md5.toUpper().append(vc_), QCryptographicHash::Md5).toHex().toUpper();
    return md5;
}

void QQLogin::login()
{
    QByteArray md5 = getPwMd5();
    QString login_url ="/login?u=" + ui->le_username_->text() + "&p=" + md5 + "&verifycode="+vc_+"&webqq_type=40&remember_uin=0&aid=46000101&login2qq=1&u1=http%3A%2F%2Fweb.qq.com%2Floginproxy.html%3Flogin2qq%3D1%26webqq_type%3D10&h=1&ptredirect=0&ptlang=2052&from_ui=1&pttype=1"
           "&dumy=&fp=loginerroralert&action=4-30-764935&mibao_css=m_web";

    Request req;
    req.create(kGet, login_url);
    req.addHeaderItem("Host", "ptlogin2.qq.com");
    req.addHeaderItem("Cookie", captcha_info_.cookie_);

    fd_->connectToHost("ptlogin2.qq.com", 80);
    connect(fd_, SIGNAL(readyRead()), this, SLOT(loginRead()));
    fd_->write(req.toByteArray());
}

char QQLogin::getResultState(const QByteArray &array)
{
    int idx = array.indexOf("ptuiCB");
    return array[idx + 8];
}

void QQLogin::loginRead()
{
    fd_->disconnectFromHost();

    QString ptwebqq;
    QByteArray result = fd_->readAll();

    char result_state = getResultState(result);

    switch (result_state)
    {
    case '0':
        break;
    case '3':
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText(tr("Password validation error!!"));
        box.setInformativeText(tr("The password is not correct, the reason may be:\nForgot password; Not case sensitive; Not open small keyboard."));

        box.exec();

        return;
    }
    case '4':
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setText(tr("Authcode error!!"));
        box.setInformativeText(tr("The Authcode is not correct! Please relogin!"));

        box.exec();
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
            captcha_info_.skey_ = value;

        captcha_info_.cookie_ = captcha_info_.cookie_ + key + "=" + value + ";";
    }

    disconnect(fd_, SIGNAL(readyRead()), this, SLOT(loginRead()));
    getLoginInfo(ptwebqq);
}

void QQLogin::loginInfoRead()
{
    QByteArray result = fd_->readAll();
    int vfwebqq_f_idx = result.indexOf("vfwebqq") + 10;
    int vfwebqq_s_idx = result.indexOf(',', vfwebqq_f_idx) - 1;
    captcha_info_.vfwebqq_ = result.mid(vfwebqq_f_idx, vfwebqq_s_idx - vfwebqq_f_idx);

    int psessionid_f_idx = result.indexOf("psessionid") + 13;
    int  psessionid_s_idx = result.indexOf(',',  psessionid_f_idx) - 1;
    captcha_info_.psessionid_ = result.mid( psessionid_f_idx,  psessionid_s_idx -  psessionid_f_idx);

    disconnect(fd_, SIGNAL(readyRead()), this, SLOT(loginInfoRead()));
    this->accept();
}

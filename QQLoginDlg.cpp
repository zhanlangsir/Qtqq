#include "QQLoginDlg.h"
#include "ui_QQLoginDlg.h"
#include "ui_captcha.h"
#include "types.h"
#include "request.h"

#include <fstream>
#include <QByteArray>
#include <QDebug>
#include <QPixmap>
#include <QCryptographicHash>
#include <QTcpSocket>
#include <assert.h>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QSettings>
#include <QMetaType>

#include "networkhelper.h"
#include "include/json/json.h"

QQLoginDlg::QQLoginDlg(QQLoginCore *login_core, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QQLoginDlg),
    login_core_(login_core),
    new_login_info_(NULL)
{
    ui->setupUi(this);

    qRegisterMetaType<LoginInfo>("LoginInfo");

    connect(ui->pb_login_, SIGNAL(clicked()), this, SLOT(onPbLoginClicked()));
    connect(ui->tb_mini_, SIGNAL(clicked()), this, SLOT(showMinimized()));
    connect(ui->tb_close_, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->cekb_autologin_, SIGNAL(clicked(bool)), this, SLOT(onCekbAutoLoginClick(bool)));
    connect(login_core_, SIGNAL(sig_loginDone(QQLoginCore::LoginResult)),
            this, SLOT(loginDone(QQLoginCore::LoginResult)));
    connect(ui->comb_username_, SIGNAL(currentIndexChanged(QString)), this, SLOT(currentUserChanged(QString)));
    connect(ui->comb_username_, SIGNAL(editTextChanged(QString)), this, SLOT(idChanged(QString)));

    setWindowOpacity(1);
    setWindowFlags(Qt::FramelessWindowHint);

    move((QApplication::desktop()->width() - this->width()) /2, (QApplication::desktop()->height() - this->height()) /2);

    setupStatus();
    ui->comb_username_->setAutoCompletion(true);
    ui->comb_username_->installEventFilter(this);

    readUsers();

    if (auto_login_id_.isEmpty())
        this->show();

    if (!auto_login_id_.isEmpty())
    {
        new_login_info_ = login_infos_[0];
        checkAccoutStatus();
    }
}

void QQLoginDlg::readUsers()
{
    std::ifstream is;
    is.open("users.json", std::ios::in);
    if (!is.is_open())
        return;

    Json::Reader reader;
    Json::Value value;

    if (!reader.parse(is, value, false))
    {
        return;
    }

    is.close();

    auto_login_id_ = QString::fromStdString(value["auto_login"].asString());

    Json::Value users = value["users"];
    for (unsigned int i = 0; i < users.size(); ++i)
    {
       LoginInfo *info = new LoginInfo;
       info->id_ =  QString::fromStdString(users[i]["id"].asString());
       info->pwd_ =  QString::fromStdString(users[i]["pwd"].asString());
       info->login_status_  = (FriendStatus)users[i]["login_status"].asInt();
       info->rem_pwd_ =  users[i]["rem_pwd"].asBool();

       login_infos_.append(info);

       ui->comb_username_->addItem( QString::fromStdString(users[i]["id"].asString()),
             QVariant::fromValue<LoginInfo*>(info));
    }
}

inline
LoginInfo* QQLoginDlg::findById(QString id) const
{
    LoginInfo *info = NULL;
    foreach(info, login_infos_)
    {
        if (info->id_ == id)
            return info;
    }
    return NULL;
}

inline
void QQLoginDlg::setUserLoginInfo(QString text)
{
    LoginInfo *info = findById(text);
    if (!info)
        return;

    ui->le_password_->setText(info->pwd_);
    ui->cekb_rem_pwd_->setChecked(info->rem_pwd_);
    ui->cekb_autologin_->setChecked((info->id_ == auto_login_id_));
    ui->cb_status_->setCurrentIndex(getStatusIndex(info->login_status_));
}

int QQLoginDlg::getStatusIndex(FriendStatus status) const
{
    for (int i = 0; i < ui->cb_status_->count(); ++i)
    {
        if (ui->cb_status_->itemData(i).value<FriendStatus>() == status)
            return i;
    }
    return -1;
}

QQLoginDlg::~QQLoginDlg()
{
    LoginInfo *info = NULL;
    foreach (info, login_infos_)
    {
        delete info;
        info = NULL;
    }

    delete new_login_info_;
    new_login_info_ = NULL;
    delete ui;
}

void QQLoginDlg::onPbLoginClicked()
{
    if (ui->comb_username_->currentText().isEmpty() || ui->le_password_->text().isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setInformativeText(tr("username or password is empty! please input them"));
        box.exec();
        return;
    }

    if (!new_login_info_)
        new_login_info_ = new LoginInfo;

    new_login_info_->id_ = ui->comb_username_->currentText();
    new_login_info_->pwd_ = ui->le_password_->text();
    new_login_info_->login_status_ = getLoginStatus();
    new_login_info_->rem_pwd_ = ui->cekb_rem_pwd_->isChecked();

    qDebug()<<"login ing...."<<endl;
    //this->hide();
    checkAccoutStatus();
}

void QQLoginDlg::loginDone(QQLoginCore::LoginResult result)
{
    switch (result)
    {
    case QQLoginCore::kSucess:
    {
        if (ui->cekb_autologin_->isChecked())
            auto_login_id_ = ui->comb_username_->currentText();

        curr_user_info_.set_id(new_login_info_->id_);
        curr_user_info_.set_status(new_login_info_->login_status_);

        LoginInfo *info = findById(new_login_info_->id_);
        if (info)
        {
            info->pwd_ = new_login_info_->pwd_;
            info->login_status_ = new_login_info_->login_status_;
            info->rem_pwd_ = new_login_info_->rem_pwd_;

            login_infos_.remove(login_infos_.indexOf(info));
            login_infos_.push_front(info);
        }
        else
        {
            ui->comb_username_->insertItem(0, new_login_info_->id_, QVariant::fromValue<LoginInfo*>(new_login_info_));
            login_infos_.push_front(new_login_info_);
        }

        saveConfig();
        this->hide();
        emit sig_loginFinish();
    }

        break;

    case QQLoginCore::kIdOrPwdWrong:
    {
        ui->cekb_autologin_->setChecked(false);

        QMessageBox box;
        box.setIcon(QMessageBox::Critical);
        box.setText(tr("Password validation error!!"));
        box.setInformativeText(tr("The password is not correct, the reason may be:\nForgot password; Not case sensitive; Not open small keyboard."));

        box.exec();
    }
        break;

    case QQLoginCore::kAuthcodeWrong:
    {
        ui->cekb_autologin_->setChecked(false);

        QMessageBox box;
        box.setIcon(QMessageBox::Critical);
        box.setText(tr("Authcode error!!"));
        box.setInformativeText(tr("The Authcode is not correct! Please relogin!"));

        box.exec();
    }
        break;

    case QQLoginCore::kUnknowErr:
        ui->cekb_autologin_->setChecked(false);
        break;

    default:
        break;
    }
}

void QQLoginDlg::onCekbAutoLoginClick(bool checked)
{
    if (checked)
        ui->cekb_rem_pwd_->setChecked(true);
}

void QQLoginDlg::currentUserChanged(QString text)
{
    setUserLoginInfo(text);
}

void QQLoginDlg::idChanged(QString text)
{
    LoginInfo *info = findById(text);
    if (!info)
    {
        ui->le_password_->clear();
        ui->cekb_rem_pwd_->setChecked(false);
        ui->cekb_autologin_->setChecked(false);
        ui->cb_status_->setCurrentIndex(0);
    }
    else
    {
        setUserLoginInfo(text);
    }
}

void QQLoginDlg::checkAccoutStatus()
{
    qDebug()<<"begin checkaccount status"<<endl;
    if (login_core_->checkState(ui->comb_username_->currentText()) == QQLoginCore::kExceptionCpaImg)
    {
        qDebug()<<"need cap img"<<endl;
        QPixmap pix = login_core_->getCapImg();
        showCapImg(pix);
    }
    else
    {
        qDebug()<<"begin login"<<endl;
        login_core_->login(new_login_info_->id_, new_login_info_->pwd_, getLoginStatus());
    }
}

void QQLoginDlg::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
  QPoint origin_pos = this->pos();

  QPoint origin_mouse_pos = QCursor::pos();
  distance_pos_ = origin_mouse_pos - origin_pos;
}

void QQLoginDlg::mouseMoveEvent(QMouseEvent *event)
{
    if (distance_pos_.isNull())
    {
        return;
    }

    this->move(event->globalPos() - distance_pos_);
}

void QQLoginDlg::mouseReleaseEvent(QMouseEvent *)
{
    distance_pos_ = QPoint(0, 0);
}

bool QQLoginDlg::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == ui->comb_username_)
    {
        if (e->type() == QEvent::KeyPress)
        {
            QKeyEvent *key_event = static_cast<QKeyEvent*>(e);
            if (key_event->key() == Qt::Key_Enter || key_event->key() == Qt::Key_Return)
            {
                return true;
            }
        }
        return false;
    }
    else
        return QDialog::eventFilter(obj, e);
}

FriendInfo QQLoginDlg::getCurrentUserInfo() const
{
    return curr_user_info_;
}

void QQLoginDlg::showCapImg(QPixmap pix)
{
    QDialog *captcha_dialog = new QDialog();
    Ui::QQCaptcha *ui = new Ui::QQCaptcha;
    ui->setupUi(captcha_dialog);
    ui->lbl_captcha_->setPixmap(pix);

    QString vc;
    if (captcha_dialog->exec())
    {
        vc = ui->le_captcha_->text().toUpper();
    }

    delete captcha_dialog;
    captcha_dialog = NULL;

    login_core_->login(new_login_info_->id_, new_login_info_->pwd_, getLoginStatus(), vc);
}

FriendStatus QQLoginDlg::getLoginStatus() const
{
    QSettings setting("options.ini", QSettings::IniFormat);
    FriendStatus status;

    if (setting.value("auto_login").toBool())
    {
        status = setting.value("login_status").value<FriendStatus>();
    }
    else
    {
        int idx = ui->cb_status_->currentIndex();
        status = ui->cb_status_->itemData(idx).value<FriendStatus>();
    }

    return status;
}

void QQLoginDlg::setupStatus()
{
    ui->cb_status_->addItem(QIcon("images/status/imonline.png"), tr("Online"), QVariant::fromValue<FriendStatus>(kOnline));
    ui->cb_status_->addItem(QIcon("images/status/Qme.png"), tr("CallMe"), QVariant::fromValue<FriendStatus>(kCallMe));
    ui->cb_status_->addItem(QIcon("images/status/away.png"), tr("Away"), QVariant::fromValue<FriendStatus>(kAway));
    ui->cb_status_->addItem(QIcon("images/status/busy.png"), tr("Busy"), QVariant::fromValue<FriendStatus>(kBusy));
    ui->cb_status_->addItem(QIcon("images/status/mute.png"), tr("Silent"), QVariant::fromValue<FriendStatus>(kSilent));
    ui->cb_status_->addItem(QIcon("images/status/invisible.png"), tr("Hidden"), QVariant::fromValue<FriendStatus>(kHidden));
}

void QQLoginDlg::saveConfig()
{
    Json::FastWriter writer;
    Json::Value login_info;
    Json::Value root;
    Json::Value users;

    LoginInfo *info = NULL;
    foreach(info, login_infos_)
    {
        login_info["id"] = info->id_.toStdString();
        login_info["pwd"] = info->rem_pwd_ ? info->pwd_.toStdString() : "";
        login_info["login_status"] = (int)info->login_status_;
        login_info["rem_pwd"] = info->rem_pwd_;
        users.append(login_info);
    }

    root["users"] = users;
    root["auto_login"] = auto_login_id_.toStdString();

    std::ofstream os;
    os.open("users.json", std::ios::out);
    os<<writer.write(root);
    os.close();
}

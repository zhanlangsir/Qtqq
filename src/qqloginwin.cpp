#include "qqloginwin.h"
#include "ui_qqloginwin.h"
#include "ui_captcha.h"

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

#include "core/types.h"
#include "core/request.h"
#include "include/json/json.h"
#include "qqtitlebar.h"
#include "core/qqskinengine.h"

QQLoginWin::QQLoginWin(QQLoginCore *login_core, QWidget *parent) :
    QQWidget(parent),
    ui(new Ui::QQLoginWin()),
    login_core_(login_core),
    curr_login_info_(NULL)
{
    ui->setupUi(contentWidget());

    setObjectName("loginWindow");
    setMouseTracking(true);
    setWindowIcon(QIcon(QQSkinEngine::instance()->getSkinRes("app_icon")));

    qRegisterMetaType<LoginInfo>("LoginInfo");

    connect(ui->pb_login, SIGNAL(clicked()), this, SLOT(onPbLoginClicked()));
    connect(ui->cekb_autologin_, SIGNAL(clicked(bool)), this, SLOT(onCekbAutoLoginClick(bool)));
    connect(login_core_, SIGNAL(sig_loginDone(QQLoginCore::LoginResult)),
            this, SLOT(loginDone(QQLoginCore::LoginResult)));
    connect(ui->comb_username_, SIGNAL(currentIndexChanged(QString)), this, SLOT(currentUserChanged(QString)));
    connect(ui->comb_username_, SIGNAL(editTextChanged(QString)), this, SLOT(idChanged(QString)));

    move((QApplication::desktop()->width() - this->width()) /2, (QApplication::desktop()->height() - this->height()) /2);

    setupStatus();
    //过滤回车键，因为发现每次在QComboBox中按回车会自动添加一个补全信息
    ui->comb_username_->installEventFilter(this);

    //读取记录的帐号信息
    readUsers();

    //如果auto_login_id为空，即无自动登陆帐号
    if (auto_login_id_.isEmpty())
        this->show();
    else
    {
        //否则，开始自动登陆,自动登陆帐号为记录中的第一个
        curr_login_info_ = login_infos_[0];
        checkAccoutStatus();
    }
}

QQLoginWin::~QQLoginWin()
{
    LoginInfo *info = NULL;
    foreach (info, login_infos_)
    {
        delete info;
        info = NULL;
    }

    delete curr_login_info_;
    curr_login_info_ = NULL;
    delete ui;
}


void QQLoginWin::readUsers()
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
LoginInfo* QQLoginWin::findById(QString id) const
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
void QQLoginWin::setUserLoginInfo(QString text)
{
    LoginInfo *info = findById(text);
    if (!info)
        return;

    ui->le_password_->setText(info->pwd_);
    ui->cekb_rem_pwd_->setChecked(info->rem_pwd_);
    ui->cekb_autologin_->setChecked((info->id_ == auto_login_id_));
    ui->cb_status_->setCurrentIndex(getStatusIndex(info->login_status_));
}

int QQLoginWin::getStatusIndex(FriendStatus status) const
{
    for (int i = 0; i < ui->cb_status_->count(); ++i)
    {
        if (ui->cb_status_->itemData(i).value<FriendStatus>() == status)
            return i;
    }
    return -1;
}

void QQLoginWin::onPbLoginClicked()
{
    if (ui->comb_username_->currentText().isEmpty() || ui->le_password_->text().isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setInformativeText(tr("username or password is empty! please input them"));
        box.exec();
        return;
    }

    //用curr_login_info记录当前登陆信息
    if (!curr_login_info_)
        curr_login_info_ = new LoginInfo;

    curr_login_info_->id_ = ui->comb_username_->currentText();
    curr_login_info_->pwd_ = ui->le_password_->text();
    curr_login_info_->login_status_ = getLoginStatus();
    curr_login_info_->rem_pwd_ = ui->cekb_rem_pwd_->isChecked();

    qDebug()<<"login ing...."<<endl;
    //this->hide();
    checkAccoutStatus();
}

void QQLoginWin::loginDone(QQLoginCore::LoginResult result)
{
    switch (result)
    {
    case QQLoginCore::kSucess:
    {
        if (ui->cekb_autologin_->isChecked())
            auto_login_id_ = ui->comb_username_->currentText();

        curr_user_info_.set_id(curr_login_info_->id_);
        curr_user_info_.set_status(curr_login_info_->login_status_);

        LoginInfo *info = findById(curr_login_info_->id_);
        if (info)
        {
            info->pwd_ = curr_login_info_->pwd_;
            info->login_status_ = curr_login_info_->login_status_;
            info->rem_pwd_ = curr_login_info_->rem_pwd_;

            login_infos_.remove(login_infos_.indexOf(info));
            login_infos_.push_front(info);
        }
        else
        {
            ui->comb_username_->insertItem(0, curr_login_info_->id_, QVariant::fromValue<LoginInfo*>(curr_login_info_));
            login_infos_.push_front(curr_login_info_);
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

void QQLoginWin::onCekbAutoLoginClick(bool checked)
{
    if (checked)
        ui->cekb_rem_pwd_->setChecked(true);
}

void QQLoginWin::currentUserChanged(QString text)
{
    setUserLoginInfo(text);
}

void QQLoginWin::idChanged(QString text)
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

void QQLoginWin::checkAccoutStatus()
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
        login_core_->login(curr_login_info_->id_, curr_login_info_->pwd_, getLoginStatus());
    }
}

bool QQLoginWin::eventFilter(QObject *obj, QEvent *e)
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
        return QQWidget::eventFilter(obj, e);
}

FriendInfo QQLoginWin::getCurrentUserInfo() const
{
    return curr_user_info_;
}

void QQLoginWin::showCapImg(QPixmap pix)
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

    login_core_->login(curr_login_info_->id_, curr_login_info_->pwd_, getLoginStatus(), vc);
}

FriendStatus QQLoginWin::getLoginStatus() const
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

void QQLoginWin::setupStatus()
{
    ui->cb_status_->addItem(QIcon(QQSkinEngine::instance()->getSkinRes("status_online")), tr("Online"), QVariant::fromValue<FriendStatus>(kOnline));
    ui->cb_status_->addItem(QIcon(QQSkinEngine::instance()->getSkinRes("status_qme")), tr("CallMe"), QVariant::fromValue<FriendStatus>(kCallMe));
    ui->cb_status_->addItem(QIcon(QQSkinEngine::instance()->getSkinRes("status_away")), tr("Away"), QVariant::fromValue<FriendStatus>(kAway));
    ui->cb_status_->addItem(QIcon(QQSkinEngine::instance()->getSkinRes("status_busy")), tr("Busy"), QVariant::fromValue<FriendStatus>(kBusy));
    ui->cb_status_->addItem(QIcon(QQSkinEngine::instance()->getSkinRes("status_mute")), tr("Silent"), QVariant::fromValue<FriendStatus>(kSilent));
    ui->cb_status_->addItem(QIcon(QQSkinEngine::instance()->getSkinRes("status_hidden")), tr("Hidden"), QVariant::fromValue<FriendStatus>(kHidden));
}

void QQLoginWin::saveConfig()
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

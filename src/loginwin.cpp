#include "loginwin.h"
#include "ui_loginwin.h"
#include "ui_captcha.h"

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
#include <QDesktopServices>
#include <QUrl>

#include "core/types.h"
#include "core/request.h"
#include <json/json.h>
#include "core/qqsetting.h"
#include "core/qqskinengine.h"

LoginWin::LoginWin(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginWin()),
    login_core_(new QQLoginCore())
{
    ui->setupUi(this);

    setObjectName("loginWindow");
    setWindowIcon(QIcon(QQSkinEngine::instance()->getSkinRes("app_icon")));

    connect(ui->pb_login, SIGNAL(clicked()), this, SLOT(onLoginBtnClicked()));
    connect(login_core_, SIGNAL(sig_loginDone(QQLoginCore::LoginResult)),
            this, SLOT(loginDone(QQLoginCore::LoginResult)));
    connect(ui->comb_username_, SIGNAL(currentIndexChanged(QString)), this, SLOT(currentUserChanged(QString)));
    connect(ui->comb_username_, SIGNAL(editTextChanged(QString)), this, SLOT(idChanged(QString)));

    move((QApplication::desktop()->width() - this->width()) /2, (QApplication::desktop()->height() - this->height()) /2);

    installEventFilter(this);

    setupStatus();
    setupAccountRecords();
}

LoginWin::~LoginWin()
{
    if ( login_core_ )
        login_core_->deleteLater();

    delete ui;
}

void LoginWin::setupStatus()
{
    ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->getSkinRes("status_online")), tr("Online"), QVariant::fromValue<FriendStatus>(kOnline));
    ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->getSkinRes("status_qme")), tr("CallMe"), QVariant::fromValue<FriendStatus>(kCallMe));
    ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->getSkinRes("status_away")), tr("Away"), QVariant::fromValue<FriendStatus>(kAway));
    ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->getSkinRes("status_busy")), tr("Busy"), QVariant::fromValue<FriendStatus>(kBusy));
    ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->getSkinRes("status_mute")), tr("Silent"), QVariant::fromValue<FriendStatus>(kSilent));
    ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->getSkinRes("status_hidden")), tr("Hidden"), QVariant::fromValue<FriendStatus>(kHidden));
}

void LoginWin::setupAccountRecords()
{
    account_manager_.readAccounts();
    QVector<AccountRecord*> accounts = account_manager_.accounts(); 

    foreach ( AccountRecord *account, accounts )
    {
       ui->comb_username_->addItem(account->id_);
    }

    this->show();
 }

inline
void LoginWin::setUserLoginInfo(QString text)
{
    AccountRecord *record = account_manager_.findAccountById(text);
    if (!record)
        return;

    ui->le_password_->setText(record->pwd_);
    ui->cekb_rem_pwd_->setChecked(record->rem_pwd_);
    ui->cb_status->setCurrentIndex(getStatusIndex(record->login_status_));
}

int LoginWin::getStatusIndex(FriendStatus status) const
{
    for (int i = 0; i < ui->cb_status->count(); ++i)
    {
        if (ui->cb_status->itemData(i).value<FriendStatus>() == status)
            return i;
    }
    return -1;
}

void LoginWin::onLoginBtnClicked()
{
    if (ui->comb_username_->currentText().isEmpty() || ui->le_password_->text().isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setInformativeText(tr("username or password is empty! please input them"));
        box.exec();
        return;
    }

    curr_login_account_.id_ = ui->comb_username_->currentText();
    curr_login_account_.pwd_ = ui->le_password_->text();
    curr_login_account_.login_status_ = getLoginStatus();
    curr_login_account_.rem_pwd_ = ui->cekb_rem_pwd_->isChecked();


    ui->pb_login->setEnabled(false);
    qDebug()<<"login ing...."<<endl;

    checkAccoutStatus();
}

void LoginWin::loginDone(QQLoginCore::LoginResult result)
{
    switch (result)
    {
    case QQLoginCore::kSucess:
    {
        curr_login_account_.pwd_ = ui->cekb_rem_pwd_->isChecked() ? ui->le_password_->text() : QString::null;

        QQSettings::instance()->currLoginInfo().id = curr_login_account_.id_;
        QQSettings::instance()->currLoginInfo().status = curr_login_account_.login_status_;

        if (ui->comb_username_->findText(curr_login_account_.id_) == -1)
        {
            ui->comb_username_->insertItem(0, curr_login_account_.id_);
        }

        account_manager_.setCurrLoginAccount(curr_login_account_);
        account_manager_.saveAccounts();

        ui->pb_login->setEnabled(true);
        this->hide();
        emit sig_loginFinish();
    }

        break;

    case QQLoginCore::kIdOrPwdWrong:
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Critical);
        box.setText(tr("Password validation error!!"));
        box.setInformativeText(tr("The password is not correct, the reason may be:\nForgot password; Not case sensitive; Not open small keyboard."));

        box.exec();
        ui->pb_login->setEnabled(true);
    }
        break;

    case QQLoginCore::kAuthcodeWrong:
    {
        QMessageBox box;
        box.setIcon(QMessageBox::Critical);
        box.setText(tr("Authcode error!!"));
        box.setInformativeText(tr("The Authcode is not correct! Please relogin!"));

        box.exec();
        ui->pb_login->setEnabled(true);
    }
        break;

    case QQLoginCore::kUnknowErr:
        ui->pb_login->setEnabled(true);
        break;

    default:
        ui->pb_login->setEnabled(true);
        break;
    }
}

void LoginWin::onAutoLoginBtnClicked(bool checked)
{
    if (checked)
        ui->cekb_rem_pwd_->setChecked(true);
}

void LoginWin::currentUserChanged(QString text)
{
    setUserLoginInfo(text);
}

void LoginWin::idChanged(QString text)
{
    AccountRecord *record = account_manager_.findAccountById(text);
    if (!record)
    {
        ui->le_password_->clear();
        ui->cekb_rem_pwd_->setChecked(false);
        ui->cb_status->setCurrentIndex(0);
    }
    else
    {
        setUserLoginInfo(text);
    }
}

void LoginWin::on_register_account_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(QUrl(link));
}

void LoginWin::on_find_password_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(QUrl(link));
}

void LoginWin::checkAccoutStatus()
{
    qDebug()<<"begin checkaccount status"<<endl;
    if (login_core_->checkState( curr_login_account_.id_ ) == QQLoginCore::kExceptionCpaImg)
    {
        qDebug()<<"need cap img"<<endl;
        QPixmap pix = login_core_->getCapImg();
        showCapImg(pix);
    }
    else
    {
        qDebug()<<"begin login"<<endl;
        login_core_->login(curr_login_account_.id_, curr_login_account_.pwd_, curr_login_account_.login_status_);
    }
}

bool LoginWin::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == ui->comb_username_ ||
		obj == this ||
		obj == ui->le_password_)
    {
        if (e->type() == QEvent::KeyPress)
        {
            QKeyEvent *key_event = static_cast<QKeyEvent*>(e);
            if (key_event->key() == Qt::Key_Enter || key_event->key() == Qt::Key_Return)
            {
				onLoginBtnClicked();
                return true;
            }
        }
        return false;
    }
    else
        return QWidget::eventFilter(obj, e);
}

void LoginWin::showCapImg(QPixmap pix)
{
    QDialog *captcha_dialog = new QDialog();
    Ui::QQCaptcha *cap_ui = new Ui::QQCaptcha;
    cap_ui->setupUi(captcha_dialog);
    cap_ui->lbl_captcha_->setPixmap(pix);

    QString vc;
    if (captcha_dialog->exec())
    {
        vc = cap_ui->le_captcha_->text().toUpper();
    }

    delete captcha_dialog;
    captcha_dialog = NULL;

    login_core_->login(curr_login_account_.id_, curr_login_account_.pwd_, getLoginStatus(), vc);
}

FriendStatus LoginWin::getLoginStatus() const
{
    QSettings setting(QQSettings::configDir() + "/options.ini", QSettings::IniFormat);
    FriendStatus status;

    if (setting.value("auto_login").toBool())
    {
        status = setting.value("login_status").value<FriendStatus>();
    }
    else
    {
        int idx = ui->cb_status->currentIndex();
        status = ui->cb_status->itemData(idx).value<FriendStatus>();
    }

    return status;
}

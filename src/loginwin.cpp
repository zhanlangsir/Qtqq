#include "loginwin.h"
#include "ui_loginwin.h"
#include "ui_captcha.h"

#include <assert.h>

#include <QApplication>
#include <QByteArray>
#include <QDesktopServices>
#include <QKeyEvent>
#include <QPixmap>
#include <QTcpSocket>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QSettings>
#include <QUrl>
#include <QDebug>

#include "json/json.h"

#include "skinengine/qqskinengine.h"
#include "protocol/qq_protocol.h"
#include "qtqq.h"

LoginWin::LoginWin(AccountManager &account_mgr, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginWin()),
    account_manager_(account_mgr)
{
    ui->setupUi(this);

    setObjectName("loginWindow");

    setWindowIcon(QIcon(QQGlobal::instance()->appIconPath()));

    connect(ui->pb_login, SIGNAL(clicked()), this, SLOT(beginLogin()));
    connect(ui->comb_username_, SIGNAL(currentIndexChanged(QString)), this, SLOT(currentUserChanged(QString)));
    connect(ui->comb_username_, SIGNAL(editTextChanged(QString)), this, SLOT(idChanged(QString)));

    move((QApplication::desktop()->width() - this->width()) /2, (QApplication::desktop()->height() - this->height()) /2);

    installEventFilter(this);

    setupStatus();
    setupAccountRecords();
}

LoginWin::~LoginWin()
{
    delete ui;
}

void LoginWin::setupStatus()
{
    ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->skinRes("status_online")), tr("Online"), QVariant::fromValue<ContactStatus>(CS_Online));
    ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->skinRes("status_qme")), tr("CallMe"), QVariant::fromValue<ContactStatus>(CS_CallMe));
    ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->skinRes("status_away")), tr("Away"), QVariant::fromValue<ContactStatus>(CS_Away));
    ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->skinRes("status_busy")), tr("Busy"), QVariant::fromValue<ContactStatus>(CS_Busy));
    ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->skinRes("status_mute")), tr("Silent"), QVariant::fromValue<ContactStatus>(CS_Silent));
    ui->cb_status->addItem(QIcon(QQSkinEngine::instance()->skinRes("status_hidden")), tr("Hidden"), QVariant::fromValue<ContactStatus>(CS_Hidden));
}

void LoginWin::setupAccountRecords()
{
    QVector<AccountRecord*> accounts = Qtqq::instance()->account_mgr_.accounts(); 

    foreach ( AccountRecord *account, accounts )
    {
        ui->comb_username_->addItem(account->id);
    }

    this->show();
}

    inline
void LoginWin::setUserLoginInfo(QString text)
{
    AccountRecord *record = account_manager_.findAccountById(text);
    if (!record)
        return;

    ui->le_password_->setText(record->pwd);
    ui->cekb_rem_pwd_->setChecked(record->rem_pwd);
    ui->cb_status->setCurrentIndex(getStatusIndex(record->login_status));
}

int LoginWin::getStatusIndex(ContactStatus status) const
{
    for (int i = 0; i < ui->cb_status->count(); ++i)
    {
        if (ui->cb_status->itemData(i).value<ContactStatus>() == status)
            return i;
    }
    return -1;
}

void LoginWin::beginLogin()
{
    if (ui->comb_username_->currentText().isEmpty() || ui->le_password_->text().isEmpty())
    {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Critical);
        box.setInformativeText(tr("username or password is empty! please input them"));
        box.exec();
        return;
    }

    QString uin = ui->comb_username_->currentText();
    QString pwd = ui->le_password_->text();
    ContactStatus status = getLoginStatus();
    bool rem_pwd = ui->cekb_rem_pwd_->isChecked();

    login(uin, pwd, status, rem_pwd);
}

void LoginWin::login(QString uin, QString pwd, ContactStatus status, bool rem_pwd)
{
    curr_login_account_.id = uin;
    curr_login_account_.pwd = pwd;
    curr_login_account_.login_status = status;
    curr_login_account_.rem_pwd = rem_pwd;

    ui->pb_login->setEnabled(false);

    qDebug() << "Begin Login" << endl;
    checkAccoutStatus(uin, pwd, status);
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

void LoginWin::checkAccoutStatus(QString uin, QString pwd, ContactStatus status)
{
    Protocol::QQProtocol *proto = Protocol::QQProtocol::instance();

    QQLoginCore::LoginResult ret;
    if (proto->checkAccountStatus(uin) == QQLoginCore::kExceptionCpaImg)
    {
        qDebug()<<"Need captcha Image"<<endl;
        QPixmap pix = proto->getCapImg();
        QString vc = showCapImg(pix);
        ret = proto->login(uin, pwd, status, vc);
    }
    else
    {
        ret = proto->login(uin, pwd, status);

    }

    switch (ret)
    {
        case QQLoginCore::kSucess:
            {
                curr_login_account_.pwd = ui->cekb_rem_pwd_->isChecked() ? ui->le_password_->text() : QString::null;

                if (ui->comb_username_->findText(curr_login_account_.id) == -1)
                {
                    ui->comb_username_->insertItem(0, curr_login_account_.id);
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
        default:
            {
                QMessageBox box;
                box.setIcon(QMessageBox::Critical);
                box.setText(tr("Unkown error!!"));
                box.setInformativeText(tr("Unknown error occur! Please relogin!"));

                ui->pb_login->setEnabled(true);
            }

            break;
    }
}

bool LoginWin::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == ui->comb_username_ || obj == ui->le_password_ ||
            obj == this)
    {
        if (e->type() == QEvent::KeyPress)
        {
            QKeyEvent *key_event = static_cast<QKeyEvent*>(e);
            if (key_event->key() == Qt::Key_Enter || key_event->key() == Qt::Key_Return)
            {
                beginLogin();
                return true;
            }
        }
        return false;
    }
    else
        return QWidget::eventFilter(obj, e);
}

void LoginWin::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    qApp->quit();
}


QString LoginWin::showCapImg(QPixmap pix)
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

    return vc;
}

ContactStatus LoginWin::getLoginStatus() const
{
    QSettings setting(QQGlobal::configDir() + "/options.ini", QSettings::IniFormat);
    ContactStatus status;

    if (setting.value("auto_login").toBool())
    {
        status = setting.value("login_status").value<ContactStatus>();
    }
    else
    {
        int idx = ui->cb_status->currentIndex();
        status = ui->cb_status->itemData(idx).value<ContactStatus>();
    }

    return status;
}

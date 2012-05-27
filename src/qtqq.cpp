#include "qtqq.h"

#include <QSettings>
#include <QMessageBox>

#include "qqloginwin.h"
#include "qqmainwindow.h"
#include "core/qqskinengine.h"
#include "core/qqsetting.h"

Qtqq::Qtqq() : login_dlg_(NULL),
    login_core_(NULL),
    mainpanel_(NULL)
{
    QQSkinEngine::instance()->updateSkin(QQSettings::instance()->skin());
}

Qtqq::~Qtqq()
{
    login_core_->deleteLater();
    if (login_dlg_)
    {
    login_dlg_->close();
    login_dlg_->deleteLater();
    }
    if (mainpanel_)
    {
    mainpanel_->close();
    mainpanel_->deleteLater();
    }
}

void Qtqq::start()
{
    login_core_ = new QQLoginCore();
    login_dlg_ = new QQLoginWin(login_core_);
    connect(login_dlg_, SIGNAL(sig_loginFinish()), this, SLOT(slot_showMainPanel()));
}

void Qtqq::slot_showMainPanel()
{
    FriendInfo user_info = login_dlg_->getCurrentUserInfo();

    mainpanel_ = new QQMainWindow(user_info);
    connect(mainpanel_, SIGNAL(sig_logout()), this, SLOT(restart()));
    mainpanel_->show();
}

void Qtqq::restart()
{
    mainpanel_->close();
    mainpanel_->deleteLater();
    mainpanel_ = NULL;

    login_dlg_->show();
}

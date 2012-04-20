#include <QApplication>
#include <QTextCodec>
#include <QtMsgHandler>
#include <QMessageBox>

#include "login.h"
#include "mainpanel.h"

int main(int argc, char *argv[])
{
    QTextCodec *codec = QTextCodec::codecForName("utf8");
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForTr(codec);
    QTextCodec::setCodecForCStrings(codec);

    QApplication a(argc, argv);

    QQLogin *login = new QQLogin;
    if (login->exec() == QDialog::Rejected)
    {
        return 0;
    }

    //system tray icon
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(0, QObject::tr("Systray"),
            QObject::tr("I couldn't detect any system tray "
                "on this system."));
    }

    CaptchaInfo captcha_info = login->getCaptchaInfo();
    FriendInfo user_info = login->getCurrentUserInfo();
    login->close();

    QQMainPanel *main_panel = new QQMainPanel(captcha_info, user_info);
    main_panel->initialize();
    main_panel->show();

    return a.exec();
}

#include <QApplication>
#include <QTextCodec>
#include <QtMsgHandler>
#include <QSettings>

#include "login.h"
#include "mainpanel.h"

//void customMessageHandler(QtMsgType type, const char *msg)
//{
//	QString txt;
//	switch (type) {
//	case QtDebugMsg:
//		txt = QString("Debug: %1").arg(msg);
//		break;
 
//	case QtWarningMsg:
//		txt = QString("Warning: %1").arg(msg);
//	break;
//	case QtCriticalMsg:
//		txt = QString("Critical: %1").arg(msg);
//	break;
//	case QtFatalMsg:
//		txt = QString("Fatal: %1").arg(msg);
//		abort();
//	}
 
//	QFile outFile("debuglog.txt");
//	outFile.open(QIODevice::WriteOnly | QIODevice::Append);
//	QTextStream ts(&outFile);
//	ts << txt << endl;
//}

int main(int argc, char *argv[])
{
    QTextCodec *codec = QTextCodec::codecForName("utf8");
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForTr(codec);
    QTextCodec::setCodecForCStrings(codec);

    QCoreApplication::setOrganizationName("FuckGFW");
    QCoreApplication::setApplicationName("Qtqq");

    QApplication a(argc, argv);
    //qInstallMsgHandler(customMessageHandler);

    QQLogin *login = new QQLogin;
    if (login->exec() == QDialog::Rejected)
    {
        return 0;
    }
    login->deleteLater();

    FriendInfo user_info = login->getCurrentUserInfo();
    login->close();

    QQMainPanel *main_panel = new QQMainPanel(user_info);
    main_panel->initialize();
    main_panel->show();

    return a.exec();
}

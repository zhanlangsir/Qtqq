#include <QApplication>
#include <QTextCodec>
#include <QtMsgHandler>
#include <QSettings>
#include <QTextStream>
#include <QFile>

#include "include/log4cplus/logger.h"
#include "include/log4cplus/loglevel.h"
#include "include/log4cplus/layout.h"
#include "include/log4cplus/fileappender.h"
#include "include/log4cplus/configurator.h"

#include "Qtqq.h"

void customMessageHandler(QtMsgType type, const char *msg)
{
    QString txt;
    switch (type) {
    case QtDebugMsg:
        txt = QString("Debug: %1").arg(msg);
        break;
 
    case QtWarningMsg:
        txt = QString("Warning: %1").arg(msg);
    break;
    case QtCriticalMsg:
        txt = QString("Critical: %1").arg(msg);
    break;
    case QtFatalMsg:
        txt = QString("Fatal: %1").arg(msg);
        abort();
    }
 
    QFile outFile("debuglog.txt");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << endl;
}

int main(int argc, char *argv[])
{
    QTextCodec *codec = QTextCodec::codecForName("utf8");
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForTr(codec);
    QTextCodec::setCodecForCStrings(codec);

    QCoreApplication::setOrganizationName("FuckGFW");
    QCoreApplication::setApplicationName("Qtqq");

    QApplication a(argc, argv);
    qInstallMsgHandler(customMessageHandler);

    log4cplus::PropertyConfigurator::doConfigure("logconf.conf");

    Qtqq app;
    app.start();

    return a.exec();
}

#include <QApplication>
#include <QTextCodec>
#include <QtMsgHandler>
#include <QSettings>
#include <QTextStream>
#include "log4qt/logger.h"
#include "log4qt/propertyconfigurator.h"
#include "log4qt/logmanager.h"

#include "qtqq.h"

int main(int argc, char *argv[])
{
    QTextCodec *codec = QTextCodec::codecForName("utf8");
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForTr(codec);
    QTextCodec::setCodecForCStrings(codec);

    QCoreApplication::setOrganizationName("FuckGFW");
    QCoreApplication::setApplicationName("Qtqq");

    QApplication a(argc, argv);

    Log4Qt::LogManager::setHandleQtMessages(true);
    Log4Qt::PropertyConfigurator::configure("log4qt.conf");

    Qtqq app;
    app.start();

    return a.exec();
}

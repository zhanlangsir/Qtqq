#include <QApplication>
#include <QTextCodec>
#include <QSettings>
#include <QTranslator>

#include "log4qt/logger.h"
#include "log4qt/propertyconfigurator.h"
#include "log4qt/logmanager.h"

#include "qtqq.h"
#include "config.h"

int main(int argc, char *argv[])
{
    QTextCodec *codec = QTextCodec::codecForName("utf8");
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForTr(codec);
    QTextCodec::setCodecForCStrings(codec);

    QCoreApplication::setOrganizationName("QtqqTeam");
    QCoreApplication::setApplicationName("Qtqq");

    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    QTranslator translator;
    QString transla_file_path = PKG_DATA_DIR"/translations";
    translator.load(QLocale::system().name(), transla_file_path);
    a.installTranslator(&translator);

    Log4Qt::LogManager::setHandleQtMessages(true);
    Log4Qt::PropertyConfigurator::configure(PKG_DATA_DIR"/log4qt.conf");

    Qtqq qtqq;
    qtqq.start();

    return a.exec();
}

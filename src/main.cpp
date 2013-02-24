#include <QApplication>
#include <QTextCodec>
#include <QSettings>
#include <QTranslator>
#include <QCleanlooksStyle>

#include "log4qt/logger.h"
#include "log4qt/propertyconfigurator.h"
#include "log4qt/logmanager.h"
#include "log4qt/appender.h"
#include "log4qt/fileappender.h"
#include "log4qt/ttcclayout.h"

#include "qtqq.h"
#include "qqglobal.h"

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
    QString transla_file_path = QQGlobal::dataDir() + "/translations";

#if QT_VERSION >= 0x040800
    translator.load(QLocale::system(), "qtqq", "_", transla_file_path, ".qm");
#else
    translator.load(transla_file_path + '/' + "qtqq_zh");
#endif
    a.installTranslator(&translator);

    Log4Qt::LogManager::setHandleQtMessages(true);
    Log4Qt::PropertyConfigurator::configure(QQGlobal::dataDir() + "/misc/log4qt.conf");

    //set up file logger, log file in ~/.Qtqq/log.log
    Log4Qt::Logger *logger = Log4Qt::Logger::rootLogger();
    Log4Qt::FileAppender *file_appender = new Log4Qt::FileAppender();
    file_appender->setName("FileAppender");
    file_appender->setFile(QQGlobal::configDir() + "/log.log");
    Log4Qt::TTCCLayout *filelayout = new Log4Qt::TTCCLayout(Log4Qt::TTCCLayout::ISO8601);
    file_appender->setLayout(filelayout);
    file_appender->activateOptions();
    logger->addAppender(file_appender);

    Qtqq::instance()->start() ;
    return a.exec();
}

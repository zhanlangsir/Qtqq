#include "qqglobal.h"

const QString QQGlobal::app_name_ = PACKAGE;
const QString QQGlobal::data_path_ = QString::fromUtf8(PKG_DATA_DIR);

QQGlobal *QQGlobal::instance_ = NULL;

QQGlobal::QQGlobal()
{
	QFile file(dataDir()+"/res.xml");
	file.open(QIODevice::ReadOnly);

	QDomDocument global_xml("xml");
	global_xml.setContent(&file);

	file.close();

	appicon_path_ = dataDir()+ "/" + global_xml.documentElement().attribute("app_icon");
}

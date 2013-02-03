#include "qqglobal.h"


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

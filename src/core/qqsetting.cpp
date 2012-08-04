#include <QDir>

#include "qqsetting.h"

QPointer<QQSettings> QQSettings::settings_;

QString QQSettings::userConfigDir() 
{
	QString dir = QDir::homePath() + "/.config";
	QDir().mkdir(dir);

	return dir;
}

QString QQSettings::configDir() 
{
	QString dir = userConfigDir() + "/"PACKAGE;
	QDir().mkdir(dir);

	return dir;
}

QString QQSettings::tempPath() 
{
	QString dir = configDir() + "/temp";
	QDir().mkdir(dir);

	return dir;
}

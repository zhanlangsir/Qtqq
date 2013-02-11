#ifndef QQGLOBAL_H
#define QQGLOBAL_H

#include <QtXml/QDomDocument>
#include <QDir>
#include <QFile>
#include <QString>

#include "config.h"

class QQGlobal
{
public:
	static QQGlobal *instance()
	{
		if ( !instance_ )
			instance_ = new QQGlobal();

		return instance_;
	}

    static QString appName() 
    {
        static const QString app_name_ = PACKAGE; 
        return app_name_;
    }

    static QString dataDir() 
    {
        static const QString data_path = QString::fromUtf8(PKG_DATA_DIR);
        return  data_path;
    }

    static QString configDir()
    {
        QString config_path = QDir::homePath() + "/.config/" + appName();

        QDir config_dir(config_path);
        if ( !config_dir.exists() )
            config_dir.mkdir(config_path);

        return config_path;
    }

    static QString pluginDir()
    {
        static const QString plugin_dir = QString::fromUtf8(PLUGIN_DIR);
        return plugin_dir;
    }

    static QString version() 
    {
        static const QString qtqq_version = VERSION;
        return qtqq_version;
    }

	static QString skinsDir() 
	{ return dataDir() + "/skins"; }
    static QString resourceDir() 
    { return dataDir() + "/res"; }
	static QString messageStyleDir()
	{
		return dataDir() + "/messagestyle"; }

    static QString tempDir() 
    {
        QString temp_path = QDir::tempPath() + '/' + appName();

        QDir temp_dir(temp_path);
        if ( !temp_dir.exists() )
            temp_dir.mkdir(temp_path);

        return temp_path;
    }

	QString appIconPath() const
	{
        return appicon_path_;
	}

private:
	QString appicon_path_;

	static const QString version_;

private:
    QQGlobal();
    QQGlobal(const QQGlobal&);
    QQGlobal& operator=(const QQGlobal&);

	static QQGlobal *instance_;
};

#endif //QQGLOBAL_H

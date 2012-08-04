#ifndef QTQQ_CORE_QQGLOBAL_H
#define QTQQ_CORE_QQGLOBAL_H


#include <QDir>
#include <QString>

#include "config.h"

class QQGlobal
{
public:
    static QString appName() 
    {
        return app_name_;
    }

    static QString configPath()
    {
        QString config_path = QDir::homePath() + "/." + appName();

        QDir config_dir(config_path);
        if ( !config_dir.exists() )
            config_dir.mkdir(config_path);

        return config_path;
    }

    static QString resourcePath() 
    { return dataPath() + "/resources"; }

    static QString tempPath() 
    {
        QString temp_path = QDir::tempPath() + '/' + appName();

        QDir temp_dir(temp_path);
        if ( !temp_dir.exists() )
            temp_dir.mkdir(temp_path);

        return temp_path;
    }

    static QString dataPath()
    {
        return data_path_;
    }

private:
    QQGlobal();
    QQGlobal(const QQGlobal&);
    QQGlobal& operator=(const QQGlobal&);

private:
    static const QString app_name_;
    static const QString data_path_;
};


#endif //QTQQ_CORE_QQGLOBAL_H

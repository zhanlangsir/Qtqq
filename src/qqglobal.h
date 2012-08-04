#ifndef QTQQ_CORE_QQGLOBAL_H
#define QTQQ_CORE_QQGLOBAL_H


#include <QDir>
#include <QString>

#include "config.h"
#include "core/singleton.h"

class QQGlobal : public Singleton<QQGlobal>
{
public:
    static QString appName() const
    {
        return app_name_;
    }

    static QString configPath() const
    {
        QString config_path = QDir::homePath() + "/." + appName();

        QDir config_dir(config_path);
        if ( !config_dir.exists() )
            config_dir.mkdir(config_path);

        return config_path;
    }

    static QString resourcePath() const
    { return dataPath() + "/resources"; }

    static QString tempPath() const
    {
        return QDir::tempPath();
    }

    static QString dataPath() const
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

const QString QQGlobal::app_name_ = PACKAGE;
const QString QQGlobal::data_path_ = PKG_DATA_DIR;

#endif //QTQQ_CORE_QQGLOBAL_H

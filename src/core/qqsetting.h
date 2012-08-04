#ifndef QTQQ_CORE_QQSETTING_H
#define QTQQ_CORE_QQSETTING_H

#include <QSettings>
#include <QPointer>
#include <QCoreApplication>
#include <QApplication>

#include "types.h"
#include "config.h"
#include "../qqglobal.h"

class QQSettings : public QSettings
{
public:
    explicit QQSettings(QString path, Format format, QObject *parent = NULL) :
        QSettings(path, format, parent)
    {
    }

    static QQSettings* instance()
    {
        if (!settings_)
		{
			QString file = configDir() + "/options.ini";
            settings_ = new QQSettings(file, QSettings::IniFormat);
		}

        return settings_;
    }

public:
    void setSkin(const QString &skin_name)
    { setValue("skin", skin_name); }
    QString skin() const
    { return value("skin", "default").toString(); }

    static QString skinsDir()
    { return PKG_DATA_DIR"/skins"; }

    QString currSkinPath() const
    { return PKG_DATA_DIR"/skins/" + value("skin", "default").toString(); }

    QString resourcePath() const
    { return QQGlobal::resourcePath(); }

    static QString configDir()
    {
        return QQGlobal::configPath();
    }

    static QString tempPath()
    {
        return QQGlobal::tempPath();
    }

    void setCurrLoginInfo(QString id, QString name, FriendStatus status, QString avatar_path)
    { 
        curr_login_.id = id;
        curr_login_.name = name;
        curr_login_.status = status;
        curr_login_.avatar_path = avatar_path;
    }
    static QString messageStylePath()
    { return PKG_DATA_DIR"/skins/messagestyle"; }

    QString currentMessageStyle() const
    { return "adium"; }

    LoginInfo& currLoginInfo()
    { return curr_login_; }

    QString loginId()
    { return curr_login_.id; }
    QString loginAvatarPath()
    { return curr_login_.avatar_path; }
    FriendStatus loginStatus()
    { return curr_login_.status; }
    QString loginName()
    { return curr_login_.name; }


private:
    static QPointer<QQSettings> settings_;

    LoginInfo curr_login_;
};

#endif // QTQQ_CORE_QQSETTING_H

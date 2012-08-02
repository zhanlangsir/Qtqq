#ifndef QTQQ_CORE_QQSETTING_H
#define QTQQ_CORE_QQSETTING_H

#include <QSettings>
#include <QPointer>
#include <QCoreApplication>
#include <QApplication>

#include "types.h"

class QQSettings : public QSettings
{
public:
    explicit QQSettings(QString path, Format format, QObject *parent = NULL) :
        QSettings(path, format, parent)
    {
        app_path_ = QCoreApplication::applicationDirPath();
        if (app_path_.endsWith("debug"))
            app_path_.replace("/debug", "");
        else if (app_path_.endsWith("release"))
            app_path_.replace("/release", "");
    }

    static QQSettings* instance()
    {
        if (!settings_)
            settings_ = new QQSettings("options.ini", QSettings::IniFormat);

        return settings_;
    }

public:
    void setSkin(const QString &skin_name)
    { setValue("skin", skin_name); }
    QString skin() const
    { return value("skin", "default").toString(); }

    QString skinsDir() const
    { return app_path_ + "/skins"; }

    QString currSkinPath() const
    { return app_path_ + "/skins/" + value("skin", "default").toString(); }

    QString resourcePath() const
    { return app_path_ + "/resources"; }

    QString tempPath() const
    { return app_path_ + "/temp"; }

    void setCurrLoginInfo(QString id, QString name, FriendStatus status, QString avatar_path)
    { 
        curr_login_.id = id;
        curr_login_.name = name;
        curr_login_.status = status;
        curr_login_.avatar_path = avatar_path;
    }

    QString messageStylePath() const
    { return app_path_ + "/skins/messagestyle"; }
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
    QString app_path_;

    LoginInfo curr_login_;
};

#endif // QTQQ_CORE_QQSETTING_H

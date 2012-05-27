#ifndef QTQQ_CORE_QQSETTING_H
#define QTQQ_CORE_QQSETTING_H

#include <QSettings>
#include <QPointer>
#include <QCoreApplication>
#include <QApplication>

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

private:
    static QPointer<QQSettings> settings_;
    QString app_path_;
};

#endif // QTQQ_CORE_QQSETTING_H

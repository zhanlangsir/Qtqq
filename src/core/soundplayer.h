#ifndef QTQQ_SOUNDPLAYER_H
#define QTQQ_SOUNDPLAYER_H

#include <QtGlobal>
#include <QSettings>
#include <QDebug>

#ifdef Q_OS_WIN32
#include <QSound>
#else
    #ifdef Q_OS_LINUX
        #include <QProcess>
    #endif
#endif

#include "qqsetting.h"

class SoundPlayer
{
public:
    enum SoundType {kMsg, kSystem, kUserType};
    ~SoundPlayer()
    { 
        delete instance_; 
        instance_ = NULL; 
    }

protected:
    SoundPlayer()
    {
    }

public:
    void play(SoundType type, QString file_name = "")
    {
        QSettings setting(QQSettings::configDir() + "/options.ini", QSettings::IniFormat);
        bool mute = setting.value("mute").toBool();
        if (mute)
            return;

        QString file_be_play;
        switch(type)
        {
        case kMsg:
            file_be_play = QQSettings::instance()->resourcePath() + "/sound/classic/msg";
            break;
        case kSystem:
            file_be_play = QQSettings::instance()->resourcePath() + "/sound/classic/system";
            break;
        case kUserType:
            file_be_play = file_name;
            break;
        }

#ifdef Q_OS_WIN32
        QSound::play(file_be_play+".mp3");
#else
    #ifdef Q_OS_LINUX
        QStringList arg(file_be_play + ".mp3");
        qDebug()<<sound_process_.state()<<endl;
        if (sound_process_.state() != QProcess::Running)
             sound_process_.start("mpg123", arg);
    #endif
#endif
    }

    static SoundPlayer* singleton()
    {
        if (!instance_)
            instance_ = new SoundPlayer();
        return instance_;
    }

private:
    static SoundPlayer* instance_;

#ifdef Q_OS_LINUX
     QProcess  sound_process_;
#endif
};

#endif // QTQQ_SOUNDPLAYER_H

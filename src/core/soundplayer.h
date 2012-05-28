#ifndef QTQQ_SOUNDPLAYER_H
#define QTQQ_SOUNDPLAYER_H

#ifdef Q_OS_WIN32
#include <QSound>
#else
    #ifdef Q_OS_LINUX
    #endif
#endif

#include <QSettings>
#include <QDebug>

#include "qqsetting.h"

class SoundPlayer
{
public:
    enum SoundType {kMsg, kSystem, kUserType};

protected:
    SoundPlayer()
    {
    }

public:
    void play(SoundType type, QString file_name = "")
    {
        QSettings setting("options.ini", QSettings::IniFormat);
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
        system("mpg123 " + file_be_play.toAscii() + ".mp3");
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
};

#endif // QTQQ_SOUNDPLAYER_H

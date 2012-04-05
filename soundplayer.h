#ifndef QTQQ_SOUNDPLAYER_H
#define QTQQ_SOUNDPLAYER_H

#ifdef Q_OS_LINUX
#include <QProcess>
#else
#include <QSound>
#endif

#include <QSettings>
#include <QDebug>

class SoundPlayer
{
public:
    enum SoundType {kMsg, kUserType};

protected:
    SoundPlayer()
    {
#ifdef Q_OS_LINUX
         sound_pro_ = new QProcess;
#endif
    }

  /*  ~SoundPlayer()
    {
#ifdef Q_OS_LINUX
        sound_pro_->waitForFinished(3000);
        delete sound_pro_;
        sound_pro_ = NULL;
#endif
    }*/

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
            file_be_play = "Sound/Classic/msg";
            break;
        case kUserType:
            file_be_play = file_name;
            break;
        }

#ifdef Q_OS_LINUX
        QStringList arg(file_be_play+".mp3");
        QProcess *sound_pro_ = new QProcess();
        sound_pro_->start("mpg123", arg);
        qDebug()<<"in linux"<<endl;
#else
        QSound::play(file_be_play+".wav");
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
          QProcess *sound_pro_;
#endif
};

#endif // QTQQ_SOUNDPLAYER_H

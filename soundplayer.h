#ifndef QTQQ_SOUNDPLAYER_H
#define QTQQ_SOUNDPLAYER_H

#include <QProcess>

class SoundPlayer
{
public:
    enum SoundType {kMsg, kUserType};
    SoundPlayer() : sound_pro(new QProcess)
    {

    }

    ~SoundPlayer()
    {
        sound_pro_->waitForFinished(3000);
        delete sound_pro_;
        sound_pro_ = NULL;
    }

public:
    void play(SoundType type, QString file_name)
    {
        QString file_be_play;
        switch(type)
        {
        case kMsg:
            file_be_play = "Sound/Classic/msg.mp3";
        case kUserType:
            file_be_play = file_name;
        }

        QStringList arg(file_be_play);
        QProcess *sound_pro_ = new QProcess();
        sound_pro_->start("mpg123", arg);
    }


private:
    QProcess *sound_pro_;
};

#endif // QTQQ_SOUNDPLAYER_H

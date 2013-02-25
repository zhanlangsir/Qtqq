#ifndef SETTING_H
#define SETTING_H

#include <QSettings>

class Setting : public QSettings
{
public:
    static Setting *instance()
    {
        if ( !instance_ )
            instance_ = new Setting();

        return instance_;
    }


private:
    Setting();
    Setting(const Setting&);
    Setting &operator=(const Setting&);

    static Setting *instance_;
};

#endif //SETTING_H

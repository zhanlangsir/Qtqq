#ifndef SETTING_H
#define SETTING_H

#include <QSettings>

#include "core/curr_login_account.h"
#include "qqglobal.h"

class Setting : public QSettings
{
public:
    static Setting *instance()
    {
        if ( !instance_ )
            instance_ = new Setting(QQGlobal::configDir() + '/' + CurrLoginAccount::id()+".ini");

        return instance_;
    }

    void reset()
    {
        if ( instance_ )
        {
            instance_->sync();

            delete instance_;
            instance_ = NULL;
        }
    }

private:
    Setting(const QString &file_name);
    Setting(const Setting&);
    Setting &operator=(const Setting&);

    static Setting *instance_;
};

#endif //SETTING_H

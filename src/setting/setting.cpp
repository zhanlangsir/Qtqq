#include "setting.h"

#include "qqglobal.h"

Setting *Setting::instance_ = NULL;

Setting::Setting() :
    QSettings(QQGlobal::configDir() + "/options.ini", QSettings::IniFormat)
{
    
}

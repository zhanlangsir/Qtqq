#include "setting.h"

Setting *Setting::instance_ = NULL;

Setting::Setting(const QString &file_name) :
    QSettings(file_name, QSettings::IniFormat)
{
    
}

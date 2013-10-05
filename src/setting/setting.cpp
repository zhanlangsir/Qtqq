#include "setting.h"

Setting *Setting::instance_ = NULL;

Setting::Setting(const QString &file_name)
{
    QFile file(file_name);
    if ( file.exists() && file.open(QIODevice::ReadOnly) )
        setContent(&file, true);
    file.close();

    if ( documentElement().isNull() )
    {
        appendChild(createElement("Settings"));
    }
}

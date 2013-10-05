#ifndef SETTING_H
#define SETTING_H

#include <QDomDocument>
#include <QFile>

#include "core/curr_login_account.h"
#include "qqglobal.h"

#define Value_Attr "value"

class Setting : public QDomDocument
{
public:
    static Setting *instance()
    {
        if ( !instance_ )
            instance_ = new Setting(QQGlobal::configDir() + '/' + CurrLoginAccount::id()+".xml");

        return instance_;
    }

    void setValue(const QString &tag, const QString &value)
    {
        QDomElement elem = documentElement().firstChildElement(tag);
        if ( elem.isNull() )
        {
            QDomElement new_elem = createElement(tag);
            new_elem.setAttribute(Value_Attr, value);
            documentElement().appendChild(new_elem);
        }
        else
        {
            elem.setAttribute(Value_Attr, value);
        }

        save();
    }

    QString value(QString tag, const QString &default_value = "")
    {
        QDomElement elem = documentElement().firstChildElement(tag);
        if ( elem.isNull() )
        {
            return default_value;
        }
        return elem.attribute(Value_Attr);
    }

    void reset()
    {
        if ( instance_ )
        {
            save();

            delete instance_;
            instance_ = NULL;
        }
    }

    void save()
    {
        if (!documentElement().isNull())
        {
            QFile file(QQGlobal::configDir() + '/' + CurrLoginAccount::id()+".xml");
            if (file.open(QFile::WriteOnly|QFile::Truncate))
            {
                file.write(toString(4).toUtf8());
                file.flush();
                file.close();
            }
        }
    }

private:
    Setting(const QString &file_name);
    Setting(const Setting&);
    Setting &operator=(const Setting&);

    static Setting *instance_;
};

#endif //SETTING_H

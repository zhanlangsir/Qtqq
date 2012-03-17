#pragma once

#include <QString>
#include <QMap>

class NameConvertor
{
public:
    QString convert(QString uin)
    {
        if (uin_to_name_.contains(uin))
            return uin_to_name_[uin];
        else
            return uin;
    }
    void addUinNameMap(QString uin, QString name)
    { uin_to_name_[uin] = name; }

private:
    QMap<QString, QString> uin_to_name_;
};

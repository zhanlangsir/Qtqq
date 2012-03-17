#pragma once

#include <map>
#include <QString>
using std::map;

class  Cookie 
{
public:
    Cookie();

public:
    void append(QString key, QString value);
    void clear();
    void earse(QString key);
    QString find(QString);
    QString toString() const;
    void getFromMessage(QString message);

private:
    std::map<QString, QString> cookies_;
};

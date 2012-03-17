#include "cookie.h"
#include <iostream>
#include <utility>
#include <cstring>

Cookie::Cookie()
{
   
}

void Cookie::getFromMessage(QString message)
{
    int idx = 0;  
    
    while ((idx = message.indexOf("Set-Cookie:", idx)) != -1) 
    {
        idx += strlen("Set-Cookie: ");

        int value_idx = message.indexOf("=", idx); 
        int fin_value_idx = message.indexOf(";", idx);

        if (fin_value_idx == (value_idx + 1)) continue;

        QString key = message.mid(idx, value_idx - idx); 
        QString value = message.mid(value_idx+1, fin_value_idx - value_idx - 1);
        cookies_[key] = value;
    }
}

void Cookie::append(QString key, QString value)
{
    cookies_.insert(std::make_pair(key, value));
}

void Cookie::clear()
{
    cookies_.clear();
}

void Cookie::earse(QString key)
{
    cookies_.erase(key);
}

QString Cookie::find(QString key)
{
    return cookies_[key];
}

QString Cookie::toString() const
{
    map<QString, QString>::const_iterator pos;

    QString str;
    for (pos =  cookies_.begin(); pos!=  cookies_.end(); ++pos)
    {
        str = str + pos->first + "=" + pos->second + "; "; 
    }

    return str;
}


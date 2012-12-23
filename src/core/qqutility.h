#ifndef QTQQ_QQUTILITY_H
#define QTQQ_QQUTILITY_H

#include <zlib.h>

#include <QString>

#include "utils/contact_status.h"

namespace QQUtility
{
inline
ContactStatus stringToStatus(QString status)
{
    if (status == "online")
    {
        return CS_Online;
    }
    else if (status == "callme")
    {
        return CS_CallMe;
    }
    else if (status == "away")
    {
        return CS_Away;
    }
    else if (status == "busy")
    {
        return CS_Busy;
    }
    else if (status == "silent")
    {
        return CS_Silent;
    }
    else if (status == "offline")
    {
        return CS_Offline;
    }
    return CS_Offline;
}

inline
QString StatusToString(ContactStatus status)
{
    switch (status)
    {
    case CS_Online:
        return "online";
    case CS_CallMe:
        return "callme";
    case CS_Away:
        return "away";
    case CS_Busy:
        return "busy";
    case CS_Silent:
        return "silent";
    case CS_Hidden:
        return "hidden";
    case CS_Offline:
        return "offline";
    default:
        break;
    }
}

int httpgzdecompress(Byte *zdata, uLong nzdata,
                 Byte *data, uLong *ndata);

int gzdecompress(const QByteArray &data, QByteArray &out);
}

#endif // QTQQ_QQUTILITY_H

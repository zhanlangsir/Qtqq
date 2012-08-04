#ifndef QTQQ_QQUTILITY_H
#define QTQQ_QQUTILITY_H

#include <zlib.h>

#include "types.h"

namespace QQUtility
{
inline
FriendStatus stringToStatus(QString status)
{
    if (status == "online")
    {
        return kOnline;
    }
    else if (status == "callme")
    {
        return kCallMe;
    }
    else if (status == "away")
    {
        return kAway;
    }
    else if (status == "busy")
    {
        return kBusy;
    }
    else if (status == "silent")
    {
        return kSilent;
    }
    else if (status == "offline")
    {
        return kOffline;
    }
    return kOffline;
}

inline
QString StatusToString(FriendStatus status)
{
    switch (status)
    {
    case kOnline:
        return "online";
    case kCallMe:
        return "callme";
    case kAway:
        return "away";
    case kBusy:
        return "busy";
    case kSilent:
        return "silent";
    case kHidden:
        return "hidden";
    case kOffline:
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

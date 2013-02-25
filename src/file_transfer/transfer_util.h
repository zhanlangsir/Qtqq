#ifndef TRANSFER_UTIL_H
#define TRANSFER_UTIL_H

#include <QString>

#define KB 1024
#define M 1048576

#define B2KB(x) (x >> 10)
#define B2M(x) (x >> 20)

namespace TransferUtil
{
    inline 
    int unitTranslation(int byte, QString &unit)
    {
        if ( byte < KB )
        {
            unit = "b";
            return byte;
        }
        else if ( KB < byte && byte < M )
        {
            unit = "kb";
            return B2KB(byte);
        }
        else
        {
            unit = "m"; 
            return B2M(byte);
        }
    }
}

#endif //TRANSFER_UTIL_H

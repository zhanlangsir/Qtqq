#ifndef QTQQ_QQITEMINFOHELPER_H
#define QTQQ_QQITEMINFOHELPER_H

#include <QTcpSocket>

class QQItemInfoHelper
{
public:
    static QByteArray getFriendInfo2(QString id);
    static QByteArray getStrangetInfo2(QString id, QString gid);
};

#endif // QTQQ_QQITEMINFOHELPER_H

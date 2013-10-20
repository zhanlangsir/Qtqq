#ifndef QTQQ_CORE_SOCKETHELPER_H
#define QTQQ_CORE_SOCKETHELPER_H

#include <QByteArray>
#include <QNetworkRequest>

#include "qqutility.h"

class QTcpSocket;

inline
void setDefaultHeaderValue(QNetworkRequest &header)
{
    header.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/535.19 (KHTML, like Gecko) Chrome/18.0.1025.3 Safari/535.19");
    header.setRawHeader("Accept", "text/html, application/xml;q=0.9, "
                    "application/xhtml+xml, image/png, "
                    "image/jpeg, image/gif, "
                    "image/x-xbitmap, */*;q=0.1");
    header.setRawHeader("Accept-Language", "en-US,zh-CN,zh;q=0.9,en;q=0.8");
    header.setRawHeader("Accept-Charset", "GBK, utf-8, utf-16, *;q=0.1");
}

void getByLength(QTcpSocket *fd, QByteArray &out);
void getByTransferEncoding(QTcpSocket *fd, QByteArray &out);
void socketReceive(QTcpSocket *fd, QByteArray &out);

#endif // QTQQ_CORE_SOCKETHELPER_H

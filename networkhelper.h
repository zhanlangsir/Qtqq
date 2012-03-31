#pragma once

#include <QByteArray>
#include <QTcpSocket>
#include <QHttpRequestHeader>

class NetWorkHelper
{
public:
    static void setDefaultHeaderValue(QHttpRequestHeader &header)
    {
        header.addValue("User-Agent", "Mozilla/5.0 (Windows NT 6.1) AppleWebKit/535.19 (KHTML, like Gecko) Chrome/18.0.1025.3 Safari/535.19");
        header.addValue("Accept", "text/html, application/xml;q=0.9, "
                        "application/xhtml+xml, image/png, "
                        "image/jpeg, image/gif, "
                        "image/x-xbitmap, */*;q=0.1");
        header.addValue("Accept-Language", "en-US,zh-CN,zh;q=0.9,en;q=0.8");
        header.addValue("Accept-Charset", "GBK, utf-8, utf-16, *;q=0.1");
    }

    static QByteArray quickReceive(QTcpSocket *fd)
    {
        QByteArray result;
        fd->waitForReadyRead();
        result.append(fd->readAll());

        int length_idx = result.indexOf("Content-Length") + 16;
        int length_end_idx = result.indexOf("\r\n", length_idx);

        QString length_str = result.mid(length_idx, length_end_idx - length_idx);
        int content_length = length_str.toInt();

        int content_idx = result.indexOf("\r\n\r\n")+4;

        while ((result.length()-content_idx) < content_length)
        {
            fd->waitForReadyRead();
            result.append(fd->readAll());
        }

        return result;
    }
};


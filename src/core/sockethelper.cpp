#include "sockethelper.h"

#include <QTcpSocket>

void getByLength(QTcpSocket *fd, QByteArray &out)
{
    int length_idx = out.indexOf("Content-Length") + 16;
    int length_end_idx = out.indexOf("\r\n", length_idx);

    QString length_str = out.mid(length_idx, length_end_idx - length_idx);
    int content_length = length_str.toInt();

    int content_idx = out.indexOf("\r\n\r\n")+4;

    while ((out.length()-content_idx) < content_length)
    {
		if ( !fd->waitForReadyRead() )
			break;
        out.append(fd->readAll());
    }
}

void getByTransferEncoding(QTcpSocket *fd, QByteArray &out)
{
    while (out.indexOf("\r\n\r\n") == -1)
    {
        fd->waitForReadyRead();
        out.append(fd->readAll());
    }

    QByteArray result = out.mid(out.indexOf("\r\n\r\n")+4);

    QByteArray encryed_content;
    int chunk_size = 0;
    bool need_read_more =false;
    while (true)
    {
        if (need_read_more)
        {
            fd->waitForReadyRead();
            result.append(fd->readAll());
        }

        if ( result.length() < chunk_size )
        {
            need_read_more = true;
            continue;
        }

        if (chunk_size == 0)
        {
            int len_idx = result.indexOf("\r\n");
            if (len_idx == -1)
            {
                need_read_more = true;
                continue;
            }

            bool ok;
            chunk_size = result.mid(0,len_idx).toInt(&ok, 16);
            result = result.mid(len_idx + 2);

            if (chunk_size == 0)
                break;
        }

        int n = result.length();
        //确保可以读取到chunked后面的\r\n终止符，否则读取chunk_size-2个字节，剩余
        //1-2字节下次再读取
        if ((n == chunk_size) || (n == chunk_size + 1))
            n = chunk_size - 2;

        int to_read = qMin(n, chunk_size);
        encryed_content.append(result.mid(0,to_read));

        if ( (chunk_size - to_read) <= 0 )
        {
            result = result.mid(chunk_size + 2);
            chunk_size = 0;
        }
        else
        {
            chunk_size -= to_read;
            need_read_more = true;
            result = result.mid(to_read);
        }
    }

    out.truncate(out.indexOf("\r\n\r\n")+4);
    QByteArray decompress_data;
    QQUtility::gzdecompress(encryed_content, decompress_data);

    out.append(decompress_data);
}

void socketReceive(QTcpSocket *fd, QByteArray &out)
{
    fd->waitForReadyRead();
    out.append(fd->readAll());

    if ( out.indexOf("Transfer-Encoding") == -1 )
        getByLength(fd, out);
    else
        getByTransferEncoding(fd, out);
}

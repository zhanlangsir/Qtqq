#include "qqmsgsender.h"

#include <QTcpSocket>

void QQMsgSender::send(const Request &req)
{
    msgs_be_send_.enqueue(req);
    msg_count_.release();
}

void QQMsgSender::run()
{
    while(true)
    {
        msg_count_.acquire();
        Request req = msgs_be_send_.dequeue();

        QTcpSocket fd;
        fd.connectToHost("d.web2.qq.com", 80);
        fd.write(req.toByteArray());

        fd.waitForReadyRead();
        fd.readAll();

        fd.close();
    }
}

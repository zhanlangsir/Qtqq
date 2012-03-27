#include "qqmsgsender.h"

#include <QTcpSocket>


QQMsgSender::~QQMsgSender()
{
}

void QQMsgSender::send(const Request &req)
{
    msgs_be_send_.enqueue(req);
    start();
}

void QQMsgSender::run()
{  
    Request req = msgs_be_send_.dequeue();

    QTcpSocket fd;
    fd.connectToHost("d.web2.qq.com", 80);
    fd.write(req.toByteArray());

    fd.waitForReadyRead();
    fd.readAll();

    fd.close();
}

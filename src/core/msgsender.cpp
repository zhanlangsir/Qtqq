#include "msgsender.h"

#include <QTcpSocket>
#include <QDebug>

void MsgSender::send(const Request &req)
{
    msgs_be_send_.enqueue(req);
    start();
}

void MsgSender::run()
{  
    while (msgs_be_send_.count() != 0)
    {
        Request req = msgs_be_send_.dequeue();

        QTcpSocket fd;
        fd.connectToHost("d.web2.qq.com", 80);
        fd.write(req.toByteArray());

        fd.waitForReadyRead();
        qDebug()<<fd.readAll()<<endl;

        emit sendDone(true, "msg send done");

        fd.close();
    }
}

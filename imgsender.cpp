#include "imgsender.h"
#include "request.h"

#include <QDateTime>
#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QTcpSocket>
#include <QDebug>
#include <QHttp>

void ImgSender::send(const QString uinque_id, const QString full_path, const QString id, const CaptchaInfo cap_info)
{
    is_sending_ = true;
    QString send_url = base_send_url_+QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());
    unique_id_ = uinque_id;
    full_path_ = full_path;
    id_ = id;
    cap_info_ = cap_info;

    QFile file(full_path);
    file.open(QIODevice::ReadOnly);
    QByteArray file_data = file.readAll();

    QByteArray boundary = "----WebKitFormBoundaryk5nH7APtIbShxvqE";// + QString(QByteArray::number(QDateTime::currentDateTime().toMSecsSinceEpoch()).toHex()).toAscii();
    QByteArray msg = createSendMsg(file_data, boundary);
    
    req_.create(kPost, send_url);
    req_.addHeaderItem("Host", host_);
    req_.addHeaderItem("Content-Length", " "+QString::number(msg.length()));
    req_.addHeaderItem("Cache-Control", "max-age=0");
    req_.addHeaderItem("Origin", "http://web.qq.com");
    req_.addHeaderItem("Content-Type", "multipart/form-data; boundary="+boundary);
    req_.addHeaderItem("Referer", "http://web.qq.com/");
    req_.addHeaderItem("Cookie", cap_info.cookie_);
    req_.addRequestContent(msg);

    start();
}

void ImgSender::run()
{
    QTcpSocket fd;

    fd.connectToHost(host_, 80);
    fd.write(req_.toByteArray(), req_.toByteArray().length());

    fd.waitForReadyRead();
    QByteArray array;
    array.append(fd.readAll());
    int length_idx = array.indexOf("Content-Length") + 16;
    int length_end_idx = array.indexOf("\r\n", length_idx);

    QString length_str = array.mid(length_idx, length_end_idx - length_idx);
    int content_length = length_str.toInt();

    int img_idx = array.indexOf("\r\n\r\n")+4;
    array = array.mid(img_idx);

    while (array.length() < content_length)
    {
        fd.waitForReadyRead();
        array.append(fd.readAll());
    }

    FileInfo file_info = parseResult(array);

    fd.close();
    emit postResult(unique_id_, file_info);
    emit sendDone(unique_id_, full_path_);
    is_sending_ = false;
}



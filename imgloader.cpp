#include "imgloader.h"
#include "request.h"

#include <QFile>
#include <QDebug>

void ImgLoader::loadFriendChatImg(const QString &file_name, const QString &to_uin, const QString &msg_id)
{
    img_name_ = file_name;
    path_ = "temp/" + file_name;
    QString get_img_url = "/channel/get_cface2?lcid="+msg_id+"&guid="+file_name+"&to="+ to_uin+ "&count=5&time=1&clientid=5412354841&psessionid="+cap_info_.psessionid_;

    host_ = "d.web2.qq.com";

    header_.create(kGet, get_img_url);
    header_.addHeaderItem("Host", host_);
    header_.addHeaderItem("Referer", "http://web.qq.com");
    header_.addHeaderItem("Cookie", cap_info_.cookie_);

    start();
}

void ImgLoader::loadGroupChatImg(const QString &file_name, const QString &gid, const QString &time)
{
    img_name_ = file_name;
    path_ = "temp/" + file_name;
    QString get_img_url = "/cgi/svr/chatimg/get?af=1&pic="+file_name+"&gid="+gid+"&time="+ time;

    host_ = "qun.qq.com";
    header_.create(kGet, get_img_url);
    header_.addHeaderItem("Host", host_);
    header_.addHeaderItem("Referer", "http://web.qq.com");
    header_.addHeaderItem("Cookie", cap_info_.cookie_);

    start();
}

void ImgLoader::run()
{
    QTcpSocket fd_;
    fd_.connectToHost(host_, 80);
    fd_.write(header_.toByteArray());

    QByteArray array;
    fd_.waitForReadyRead();

    array.append(fd_.readAll());

    int idx = array.indexOf("http://") + 7;
    int end_idx = array.indexOf("\r\n", idx);
    QByteArray url = array.mid(idx, end_idx - idx);

    int host_end_idx = url.indexOf("com")+3;
    QString host = url.mid(0, host_end_idx);

    QString file_url = url.mid(host_end_idx);
    fd_.close();

    Request req;
    req.create(kGet, file_url);
    req.addHeaderItem("Host", host);
    req.addHeaderItem("Referer", "http://web.qq.com");
    req.addHeaderItem("Cookie", cap_info_.cookie_);
    fd_.connectToHost(host, 80);
    fd_.write(req.toByteArray());
    array.clear();

    fd_.waitForReadyRead();
    array.append(fd_.readAll());

    int length_idx = array.indexOf("Content-Length") + 16;
    int length_end_idx = array.indexOf("\r\n", length_idx);

    QString length_str = array.mid(length_idx, length_end_idx - length_idx);
    int content_length = length_str.toInt();

    int img_idx = array.indexOf("\r\n\r\n")+4;
    array = array.mid(img_idx);

    while (array.length() < content_length)
    {
        fd_.waitForReadyRead();
        array.append(fd_.readAll());
    }

    QFile file(path_);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    out.writeRawData(array.data(), array.length());
    file.close();

    emit loadDone(img_name_, path_);
}


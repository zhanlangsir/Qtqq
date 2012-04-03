#include "imgloader.h"
#include "request.h"

#include <QFile>
#include <QDebug>

void ImgLoader::loadFriendCface(const QString &file_name, const QString &to_uin, const QString &msg_id)
{
    LoadInfo info;

    info.img_name_ = file_name;
    info.path_ = "temp/" + file_name;
    info.url_ = "/channel/get_cface2?lcid="+msg_id+"&guid="+file_name+"&to="+ to_uin+
            "&count=5&time=1&clientid=5412354841&psessionid="+CaptchaInfo::singleton()->psessionid();
    info.host_ = "d.web2.qq.com";

    info.header_.create(kGet, info.url_);
    info.header_.addHeaderItem("Host", info.host_);
    info.header_.addHeaderItem("Referer", "http://web.qq.com");
    info.header_.addHeaderItem("Cookie", CaptchaInfo::singleton()->cookie());

    infos_.enqueue(info);

    img_count_.release();
}

void ImgLoader::loadFriendOffpic(const QString &file_name, const QString &to_uin)
{
    LoadInfo info;
    info.img_name_ = file_name;
    info.path_ = "temp/" + file_name;

    info.url_ = "/channel/get_offpic2?file_path=" +file_name + "&f_uin=" + to_uin + "&clientid=5412354841&psessionid="+
            CaptchaInfo::singleton()->psessionid();
    info.host_ = "d.web2.qq.com";

    info.header_.create(kGet, info.url_);
    info.header_.addHeaderItem("Host", info.host_);
    info.header_.addHeaderItem("Referer", "http://web.qq.com");
    info.header_.addHeaderItem("Cookie", CaptchaInfo::singleton()->cookie());

    infos_.enqueue(info);

    img_count_.release();
}


void ImgLoader::loadGroupChatImg(const QString &file_name, const QString &gid, const QString &time)
{
    LoadInfo info;
    info.img_name_ = file_name;
    info.path_ = "temp/" + file_name;
    info.url_ = "/cgi/svr/chatimg/get?af=1&pic="+file_name+"&gid="+gid+"&time="+ time;
    info.host_ = "qun.qq.com";
    info.header_.create(kGet, info.url_);
    info.header_.addHeaderItem("Host", info.host_);
    info.header_.addHeaderItem("Referer", "http://web.qq.com");
    info.header_.addHeaderItem("Cookie", CaptchaInfo::singleton()->cookie());

    infos_.enqueue(info);

    img_count_.release();
}

void ImgLoader::run()
{
    while(true)
    {
        img_count_.acquire();
        LoadInfo info = infos_.dequeue();
        QTcpSocket fd;
        fd.connectToHost(info.host_, 80);
        fd.write(info.header_.toByteArray());

        QByteArray array;
        fd.waitForReadyRead();

        array.append(fd.readAll());

        int idx = array.indexOf("http://") + 7;
        int end_idx = array.indexOf("\r\n", idx);
        QByteArray url = array.mid(idx, end_idx - idx);

        int host_end_idx = url.indexOf("com");

        QString host;
        if (host_end_idx == -1)
        {
            host_end_idx  = url.indexOf(":");
            host = url.mid(0, host_end_idx);
        }
        else
        {
            host = url.mid(0, host_end_idx+3);
        }

        int file_idx = url.indexOf("/");
        QString file_url = url.mid(file_idx);
        fd.close();

        Request req;
        req.create(kGet, file_url);
        req.addHeaderItem("Host", host);
        req.addHeaderItem("Referer", "http://web.qq.com");
        req.addHeaderItem("Cookie", CaptchaInfo::singleton()->cookie());
        fd.connectToHost(host, 80);
        fd.write(req.toByteArray());
        array.clear();

        fd.waitForReadyRead();
        array.append(fd.readAll());

        int length_idx = array.indexOf("Content-Length") + 16;
        int length_end_idx = array.indexOf("\r\n", length_idx);

        QString length_str = array.mid(length_idx, length_end_idx - length_idx);
        int content_length = length_str.toInt();


        int img_idx = array.indexOf("\r\n\r\n")+4;

        array = array.mid(img_idx);

        while (array.length() < content_length)
        {
            if (fd.waitForReadyRead() == false)
                break;

            array.append(fd.readAll());
        }

        QFile file(info.path_);
        file.open(QIODevice::WriteOnly);
        QDataStream out(&file);
        out.writeRawData(array.data(), array.length());
        file.close();

        fd.close();

        emit loadDone(info.img_name_, info.path_);
    }
}


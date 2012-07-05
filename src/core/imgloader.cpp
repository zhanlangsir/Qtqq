#include "imgloader.h"
#include "request.h"

#include <QFile>
#include <QDebug>
#include <string>
#include <QDir>

#include "captchainfo.h"

void ImgLoader::loadFriendCface(const QString &uuid, const QString &file_name, const QString &to_uin, const QString &msg_id)
{
    LoadInfo info;

    info.img_name = file_name;
    info.uuid = uuid;
    info.path = "temp/" + file_name;
    info.url = "/channel/get_cface2?lcid="+msg_id+"&guid="+file_name+"&to="+ to_uin+
            "&count=5&time=1&clientid=5412354841&psessionid="+CaptchaInfo::instance()->psessionid();
    info.host = "d.web2.qq.com";

    lock_.lock();
    infos_.enqueue(info);
    lock_.unlock();

    start();
}

void ImgLoader::loadFriendOffpic(const QString &uuid, const QString &file_name, const QString &to_uin)
{
    LoadInfo info;
    info.img_name = file_name;
    info.uuid = uuid;
    info.path = "temp/" + file_name;

    info.url = "/channel/get_offpic2?file_path=" +file_name + "&f_uin=" + to_uin + "&clientid=5412354841&psessionid="+
            CaptchaInfo::instance()->psessionid();
    info.host = "d.web2.qq.com";   

    lock_.lock();
    infos_.enqueue(info);
    lock_.unlock();

    start();
}

void ImgLoader::loadGroupChatImg(const QString &file_name, QString uin, const QString &gcode,
                                 QString fid, QString rip, QString rport, const QString &time)
{
    LoadInfo info;
    info.img_name = file_name;
    info.path = "temp/" + file_name;
    info.url = "/cgi-bin/get_group_pic?type=0&gid=" + gcode + "&uin=" + uin + "&rip=" +rip + "&rport=" + rport + "&fid=" + fid + "&pic=" + file_name + "&vfwebqq="+ CaptchaInfo::instance()->vfwebqq() + "&t="+time;
    info.host = "web.qq.com";

    lock_.lock();
    infos_.enqueue(info);
    lock_.unlock();

    start();
}

void ImgLoader::run()
{
    while(true)
    {
        lock_.lock();
        int to_load_count = infos_.count() ;
        lock_.unlock();

        if (to_load_count == 0)
            break;

        const LoadInfo info = infos_.dequeue();

        QByteArray array = getImgUrl(info);
        qDebug()<<"first revice"<<array<<endl;

        int idx = array.indexOf("http://") + 7;
        int end_idx = array.indexOf("\r\n", idx);
        QByteArray url = array.mid(idx, end_idx - idx);

        QByteArray img_data = requestImgData(getHost(url), getRequestUrl(url));

        if (img_data.isEmpty())
        {
            qDebug()<<"get img failed!"<<endl;
            continue;
        }

        saveImg(img_data, info.path);

        qDebug()<<"save img  [ "<<info.img_name << " ] to :"<<info.path<<endl;
        QDir save_path(info.path);
        emit loadDone(info.uuid, save_path.absolutePath());
    }
}

QByteArray ImgLoader::getImgUrl(const LoadInfo &info) const
{
    Request req;
    req.create(kGet, info.url);
    req.addHeaderItem("Host", info.host);
    req.addHeaderItem("Referer", "http://web.qq.com");
    QString c = CaptchaInfo::instance()->cookie().mid(0, CaptchaInfo::instance()->cookie().length()-2);
    qDebug()<<c<<endl;
    req.addHeaderItem("Cookie", c);

    QTcpSocket fd;
    fd.connectToHost(info.host, 80);
    fd.write(req.toByteArray());

    qDebug()<<"img request content:\n"<<req.toByteArray()<<endl;

    QByteArray array;
    while (array.indexOf("\r\n\r\n") == -1 && fd.waitForReadyRead(4000))
    {      
        array.append(fd.readAll());
        qDebug()<<"getting img url, revice data:\n"<<array<<endl;
    }
    fd.close();
    return array;
}

void ImgLoader::saveImg(const QByteArray &array, QString path)
{
    QFile file(path);
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);
    out.writeRawData(array.data(), array.length());
    file.close();
}

QByteArray ImgLoader::requestImgData(QString host, QString request_url)
{
    Request req;
    req.create(kGet, request_url);
    req.addHeaderItem("Host", host);
    req.addHeaderItem("Referer", "http://web.qq.com");
    req.addHeaderItem("Cookie", CaptchaInfo::instance()->cookie());

    QTcpSocket fd;
    fd.connectToHost(host, 80);
    fd.write(req.toByteArray());

    fd.waitForReadyRead();
    QByteArray img_data;
    img_data.append(fd.readAll());

    int length_idx = img_data.indexOf("Content-Length") + 16;
    int length_end_idx = img_data.indexOf("\r\n", length_idx);

    QString length_str = img_data.mid(length_idx, length_end_idx - length_idx);
    int content_length = length_str.toInt();


    int img_idx = img_data.indexOf("\r\n\r\n")+4;

    img_data = img_data.mid(img_idx);

    while (img_data.length() < content_length)
    {
        if (fd.waitForReadyRead() == false)
            break;

        img_data.append(fd.readAll());
    }
    fd.close();
    return img_data;
}

QString ImgLoader::getHost(const QByteArray &url) const
{
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
    return host;
}

QString ImgLoader::getRequestUrl(const QByteArray &url) const
{
    int file_idx = url.indexOf("/");
    return url.mid(file_idx);
}


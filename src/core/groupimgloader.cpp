#include "groupimgloader.h"

#include "captchainfo.h"

 GroupImgLoader:: GroupImgLoader()
{
}

QByteArray  GroupImgLoader::getImgUrl(const LoadInfo &info) const
{
    Request req;
    req.create(kGet, info.url);
    req.addHeaderItem("Host", info.host);
    req.addHeaderItem("Referer", "http://web.qq.com");
    req.addHeaderItem("Cookie", CaptchaInfo::instance()->cookie());

    QTcpSocket fd;
    fd.connectToHost(info.host, 80);
    fd.write(req.toByteArray());

    qDebug()<<"img request content:\n"<<req.toByteArray()<<endl;

    QByteArray array;
    while (array.indexOf("\r\n\r\n") == -1 && fd.waitForReadyRead(4000))
    {
        array.append(fd.readAll());
        qDebug()<<"getting group img url first, reviced data:\n"<<array<<endl;
    }

    fd.close();

    int idx = array.indexOf("http://") + 7;
    int end_idx = array.indexOf("\r\n", idx);
    QByteArray url = array.mid(idx, end_idx - idx);


    Request move_temp_req;
    move_temp_req.create(kGet, getRequestUrl(url));
    move_temp_req.addHeaderItem("Host", getHost(url));
    move_temp_req.addHeaderItem("Referer", "http://web.qq.com");
    move_temp_req.addHeaderItem("Cookie", CaptchaInfo::instance()->cookie());

    this->usleep(500);
    QTcpSocket move_temp_fd;
    move_temp_fd.connectToHost(getHost(url), 80);
    move_temp_fd.write(move_temp_req.toByteArray());

    QByteArray result;
    while (result.indexOf("\r\n\r\n") == -1 && move_temp_fd.waitForReadyRead(8000))
    {
        result.append(move_temp_fd.readAll());
        qDebug()<<"getting group img url second, reviced data:\n"<<result<<endl;
    }
    move_temp_fd.close();

    return result;
}

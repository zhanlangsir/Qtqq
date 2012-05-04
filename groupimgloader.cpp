#include "groupimgloader.h"

 GroupImgLoader:: GroupImgLoader()
{
}

QByteArray  GroupImgLoader::getImgUrl(const LoadInfo &info) const
{
    Request req;
    req.create(kGet, info.url);
    req.addHeaderItem("Host", info.host);
    req.addHeaderItem("Referer", "http://web.qq.com");
    req.addHeaderItem("Cookie", CaptchaInfo::singleton()->cookie());

    QTcpSocket fd;
    fd.connectToHost(info.host, 80);
    fd.write(req.toByteArray());

    qDebug()<<"img request content:\n"<<req.toByteArray()<<endl;

    QByteArray array;
    while (array.indexOf("\r\n\r\n") == -1 && fd.waitForReadyRead(4000))
    {
        array.append(fd.readAll());
    }
    fd.close();

    int idx = array.indexOf("http://") + 7;
    int end_idx = array.indexOf("\r\n", idx);
    QByteArray url = array.mid(idx, end_idx - idx);

    Request req1;
    req1.create(kGet, getRequestUrl(url));
    req1.addHeaderItem("Host", getHost(url));
    req1.addHeaderItem("Referer", "http://web.qq.com");
    req1.addHeaderItem("Cookie", CaptchaInfo::singleton()->cookie());
    fd.connectToHost(getHost(url), 80);
    fd.write(req1.toByteArray());

    array.clear();
    while (array.indexOf("\r\n\r\n") == -1 && fd.waitForReadyRead(4000))
    {
        array.append(fd.readAll());
    }
    fd.close();
    return array;
}

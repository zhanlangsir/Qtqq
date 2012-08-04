#include "qqiteminfohelper.h"

#include <QDateTime>

#include "core/request.h"
#include "core/captchainfo.h"

QByteArray QQItemInfoHelper::getFriendInfo2(QString id)
{
    QString get_friend_info_url = "/api/get_friend_info2?tuin="+ id +"&verifysession=&code=&vfwebqq=" +
            CaptchaInfo::instance()->vfwebqq() + "&t=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());

    Request req;
    req.create(kGet, get_friend_info_url);
    req.addHeaderItem("Host", "s.web2.qq.com");
    req.addHeaderItem("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001&callback=1&id=1");
    req.addHeaderItem("Connection", "keep-live");
    req.addHeaderItem("Content-Type","utf-8");
    req.addHeaderItem("Cookie", CaptchaInfo::instance()->cookie());

    QTcpSocket fd;
    fd.connectToHost("s.web2.qq.com", 80);

    fd.write(req.toByteArray());
    fd.waitForReadyRead();
    QByteArray result = fd.readAll();
    fd.close();

    return result;
}

QByteArray QQItemInfoHelper::getStrangetInfo2(QString id, QString gid)
{
    QString get_stranger_info_url = "/api/get_stranger_info2?tuin=" + id + "&verifysession=&gid=0&code=group_request_join-"+gid+"&vfwebqq=" +
            CaptchaInfo::instance()->vfwebqq() + "&t=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());

    Request req;
    req.create(kGet, get_stranger_info_url);
    req.addHeaderItem("Host", "s.web2.qq.com");
    req.addHeaderItem("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001&callback=1&id=1");
    req.addHeaderItem("Connection", "keep-live");
    req.addHeaderItem("Content-Type","utf-8");
    req.addHeaderItem("Cookie", CaptchaInfo::instance()->cookie());

    QTcpSocket fd;
    fd.connectToHost("s.web2.qq.com", 80);

    fd.write(req.toByteArray());
    fd.waitForReadyRead();
    QByteArray result = fd.readAll();
    fd.close();

    return result;
}

QByteArray QQItemInfoHelper::getSingleLongNick(QString id)
{
    QString single_long_nick_url = "/api/get_single_long_nick2?tuin=" + id+ "&vfwebqq=" + CaptchaInfo::instance()->vfwebqq() + "&t=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());

    Request req;
    req.create(kGet, single_long_nick_url);
    req.addHeaderItem("Host", "s.web2.qq.com");
    req.addHeaderItem("Referer", "http://s.web2.qq.com/proxy.html?v=20110412001&callback=1&id=1");
    req.addHeaderItem("Connection", "keep-live");
    req.addHeaderItem("Content-Type","utf-8");
    req.addHeaderItem("Cookie", CaptchaInfo::instance()->cookie());

    QTcpSocket fd;
    fd.connectToHost("s.web2.qq.com", 80);

    fd.write(req.toByteArray());
    fd.waitForReadyRead();
    QByteArray result = fd.readAll();
    fd.close();

    return result;
}

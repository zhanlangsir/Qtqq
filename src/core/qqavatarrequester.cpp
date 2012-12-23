#include "qqavatarrequester.h"

#include <QFile>
#include <QTcpSocket>
#include <QDir>

#include "request.h"
#include "captchainfo.h"
#include "sockethelper.h"
#include "qqglobal.h"

bool QQAvatarRequester::isRequesting(QString id) const
{
    if (requesting_list_.contains(id))
        return true;

    return false;
}

void QQAvatarRequester::request(QQItem *item)
{
    to_request_.enqueue(item);
    requesting_list_.append(item->id());

    start();
}

void QQAvatarRequester::finishRequest()
{
    finish_ = true;
}


//type = 1, request friend or stranger avatar
//type = 2, request group avatar
QString QQAvatarRequester::requestOne(int type, QString id, QString save_path)
{
    QTcpSocket fd;
    QString avatar_url = "/cgi/svr/face/getface?cache=0&type=%1&fid=0&uin=%2&vfwebqq=%3";

    Request req;
    req.create(kGet, avatar_url.arg(type).arg(id).arg(CaptchaInfo::instance()->vfwebqq()));
    req.addHeaderItem("Host", "face1.qun.qq.com");
    req.addHeaderItem("Referer", "http://web.qq.com");
    req.addHeaderItem("Cookie", CaptchaInfo::instance()->cookie());

    fd.connectToHost("face1.qun.qq.com", 80);
    fd.write(req.toByteArray());

    QByteArray result;
    socketReceive(&fd,result);
    
    int content_idx = result.indexOf("\r\n\r\n")+4;

    int format_idx = result.indexOf("image/") + 6;
    int format_end_idx = result.indexOf("\r\n", format_idx);

    QString format = result.mid(format_idx, format_end_idx - format_idx);
    QString save_full_path = save_path + '/' + id + "." + format;

    QFile file(save_full_path);
    file.open(QIODevice::WriteOnly);
    file.write(result.mid(content_idx));
    file.close();
    fd.close();

    return save_full_path;
}

void QQAvatarRequester::run()
{
    while (to_request_.count() != 0 && (!finish_))
    {
        QQItem *item = to_request_.dequeue(); 

        QString id;
        if (item->type() == QQItem::kFriend)
            id = item->id();
        else
        {
            id = item->gCode();
        }
        QString fold_path = QQGlobal::tempDir() + "/avatar/";
		QDir().mkdir(fold_path);
        QString full_path = requestOne(getTypeNumber(item->type()), id, fold_path);

        item->set_avatarPath(full_path);
        requesting_list_.removeOne(item->id());
    }
}

int QQAvatarRequester::getTypeNumber(QQItem::ItemType type)
{
    if (type == QQItem::kFriend)
    {
        return 1;
    }
    else if (type == QQItem::kGroup)
    {
        return 4;
    }
}

#include "qqavatarrequester.h"

#include <QFile>

#include "networkhelper.h"
#include "request.h"

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

void QQAvatarRequester::run()
{
    while (to_request_.count() != 0)
    {
        QTcpSocket fd;
        QQItem *item = to_request_.dequeue();

        QString avatar_url = "/cgi/svr/face/getface?cache=0&type=%1&fid=0&uin=%2&vfwebqq=%3";

        Request req;
        if (item->type() == QQItem::kFriend)
            req.create(kGet, avatar_url.arg(getTypeNumber(item)).arg(item->id()).arg(CaptchaInfo::singleton()->vfwebqq()));
        else
        {
            GroupInfo *info = static_cast<GroupInfo*>(item->itemInfo());
            req.create(kGet, avatar_url.arg(getTypeNumber(item)).arg(info->code()).arg(CaptchaInfo::singleton()->vfwebqq()));
        }
        req.addHeaderItem("Host", "face1.qun.qq.com");
        req.addHeaderItem("Referer", "http://web.qq.com");
        req.addHeaderItem("Cookie", CaptchaInfo::singleton()->cookie());
    
        fd.connectToHost("face1.qun.qq.com", 80);
        fd.write(req.toByteArray());

        QByteArray result = NetWorkHelper::quickReceive(&fd);

        QString save_path = "temp/avatar/"+item->id()+"."+getFileFormat(result);

        int content_idx = result.indexOf("\r\n\r\n")+4;
        QFile file(save_path);
        file.open(QIODevice::WriteOnly);
        file.write(result.mid(content_idx));
        file.close();
        fd.close();

        item->set_avatarPath(save_path);
        requesting_list_.removeOne(item->id());
    }
}

inline
QString QQAvatarRequester::getFileFormat(const QByteArray &array) const
{
    int format_idx = array.indexOf("image/") + 6;
    int format_end_idx = array.indexOf("\r\n", format_idx);
    return array.mid(format_idx, format_end_idx - format_idx);
}

inline
int QQAvatarRequester::getTypeNumber(const QQItem *item) const
{
    if (item->type() == QQItem::kFriend)
    {
        return 1;
    }
    else if (item->type() == QQItem::kGroup)
    {
        return 4;
    }
}

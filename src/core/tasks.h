#ifndef TASKS_H
#define TASKS_H

#include <QRunnable>
#include <QTcpSocket>
#include <QString>
#include <QFile>

#include "../qqitemmodel.h"
#include "captchainfo.h"
#include "request.h"
#include "qqitem.h"
#include "sockethelper.h"
#include "../qqglobal.h"

class GetAvatarTask : public QRunnable
{
public:
    GetAvatarTask(QQItem *item, QQItemModel *model) : item_(item), model_(model)
    {
    }

    static int getTypeNumber(QQItem::ItemType type)
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

    static QString getRequestId(QQItem *item)
    {
		if (item->type() == QQItem::kFriend)
        {
            return item->id(); 
        }
		else if (item->type() == QQItem::kGroup)
        {
            return item->gCode();
        }
    }

protected:
    void run()
    {
        QTcpSocket fd;
        QString avatar_url = "/cgi/svr/face/getface?cache=0&type=%1&fid=0&uin=%2&vfwebqq=%3";

        Request req;
		req.create(kGet, avatar_url.arg(getTypeNumber(item_->type())).arg(getRequestId(item_)).arg(CaptchaInfo::instance()->vfwebqq()));
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
        QString save_full_path = QQGlobal::tempPath() + '/' + item_->id() + "." + format;

        QFile file(save_full_path);
        file.open(QIODevice::WriteOnly);
        file.write(result.mid(content_idx));
        file.close();
        fd.close();

        item_->set_avatarPath(save_full_path);
        model_->notifyItemDataChanged(item_);
    }

private:
    QQItem *item_;
    QQItemModel *model_;
};

#endif //TASKS_H

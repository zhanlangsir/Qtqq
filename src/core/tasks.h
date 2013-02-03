#ifndef TASKS_H
#define TASKS_H

#include <QRunnable>
#include <QTcpSocket>
#include <QString>
#include <QFile>
#include <QPointer>

#include "../qqitemmodel.h"
#include "captchainfo.h"
#include "request.h"
#include "qqitem.h"
#include "sockethelper.h"
#include "../qqglobal.h"
#include "talkable.h"

/*
#include <cstring>
using std::string;

class Task
{
public:
    enum TaskPriority { kLow, kMiddle, kHigh };
    enum TaskType { kAvatar, kLongNick };
    virtual string identify() = 0;
    virtual void run() = 0;
};

class GetAvatarTask : public Task
{
public:
    GetAvatarTask(TaskPriority pri, TaskType type) : 
        priority_(pri),
        type_(type)
    {

    }

public:
    string identify()
    {
        char priority_str[5];
        sprintf(priority_str, "%d", priority_);
        string result(priority_str);
        result.append(item->id().toStdString());
        result.append(type_);
        return result;
    }

    void run()
    {

    }

private:
    TaskPriority priority_; 
    TaskType type_;
}:
*/

class GetAvatarTask : public QRunnable
{
public:
    GetAvatarTask(Talkable *talkable, QQItemModel *model) : talkable_(talkable), model_(model)
    {
    }
    GetAvatarTask(QQItem *item, QQItemModel *model) 
	{
	}

    static int getTypeNumber(Talkable::TalkableType type)
    {
        if (type == Talkable::kContact)
        {
            return 1;
        }
        else if (type == Talkable::kGroup)
        {
            return 4;
        }
    }

    static QString getRequestId(Talkable *talkable)
    {
		if (talkable->type() == Talkable::kContact)
        {
            return talkable->id(); 
        }
		else if (talkable->type() == Talkable::kGroup)
        {
            return talkable->gcode();
        }
    }

protected:
    void run()
    {
        QTcpSocket fd;
        QString avatar_url = "/cgi/svr/face/getface?cache=0&type=%1&fid=0&uin=%2&vfwebqq=%3";

        Request req;
		req.create(kGet, avatar_url.arg(getTypeNumber(talkable_->type())).arg(getRequestId(talkable_)).arg(CaptchaInfo::instance()->vfwebqq()));
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
        QString save_full_path = QQGlobal::tempDir() + '/' + talkable_->id() + "." + format;

        QFile file(save_full_path);
        file.open(QIODevice::WriteOnly);
        file.write(result.mid(content_idx));
        file.close();
        fd.close();

        if ( !model_.isNull() )
        {
            talkable_->setAvatarPath(save_full_path);
        }
    }

private:
    Talkable *talkable_;
    QPointer<QQItemModel> model_;
};

#endif //TASKS_H

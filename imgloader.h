#pragma once

#include "qqmsg.h"
#include "types.h"
#include "request.h"

#include <QThread>
#include <QTcpSocket>
#include <QSemaphore>
#include <QQueue>

struct LoadInfo
{
    QString path_;
    QString img_name_;
    Request header_;
    QString host_;
    QString url_;
};

class ImgLoader : public QThread
{
    Q_OBJECT
public:
    void loadFriendChatImg(const QString &file_name, const QString &to_uin, const QString &msg_id);
    void loadGroupChatImg(const QString &file_name, const QString &gid, const QString &time);
    void setCaptchaInfo(CaptchaInfo cap_info)
    { cap_info_ = cap_info; }

signals:
    void loadDone(const QString &img_name_, const QString &path);

protected:
    void run();

private:
    QSemaphore img_count_;
    QQueue<LoadInfo> infos_;
    CaptchaInfo cap_info_;
    bool has_error_;
};

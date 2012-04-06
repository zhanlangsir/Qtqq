#pragma once

#include "qqmsg.h"
#include "types.h"
#include "request.h"

#include <QThread>
#include <QTcpSocket>
#include <QQueue>
#include <QMutex>

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
    void loadFriendOffpic(const QString &file_name, const QString &to_uin);
    void loadFriendCface(const QString &file_name, const QString &to_uin, const QString &msg_id);
    void loadGroupChatImg(const QString &file_name, const QString &gid, const QString &time);

signals:
    void loadDone(const QString &img_name_, const QString &path);

protected:
    void run();

private:
    QMutex lock_;
    QQueue<LoadInfo> infos_;
    bool has_error_;
};

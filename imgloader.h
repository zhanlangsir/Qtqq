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
    QString path;
    QString img_name;
    QString host;
    QString url;
};

class ImgLoader : public QThread
{
    Q_OBJECT
public:
    void loadFriendOffpic(const QString &file_name, const QString &to_uin);
    void loadFriendCface(const QString &file_name, const QString &to_uin, const QString &msg_id);
    void loadGroupChatImg(const QString &file_name, QString uin,
                          const QString &gcode, QString fid, QString rip,
                          QString rport, const QString &time);

signals:
    void loadDone(const QString &img_name_, const QString &path);

protected:
    void run();
    virtual QByteArray getImgUrl(const LoadInfo &info) const;
    void saveImg(const QByteArray &array, QString path);
    QByteArray requestImgData(QString host, QString request_url);
    QString getHost(const QByteArray &url) const;
    QString getRequestUrl(const QByteArray &url) const;

protected:
    QQueue<LoadInfo> infos_;

private:
    QMutex lock_;
    bool has_error_;
};

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
    QString uuid;
};

class ImgLoader : public QThread
{
    Q_OBJECT

signals:
    void loadDone(const QString &img_name, const QString &path);

public:
    ImgLoader()
    {
        setTerminationEnabled(true);
    }


    void loadFriendOffpic(const QString &uuid, const QString &file_name, const QString &to_uin);
    void loadFriendCface(const QString &uuid, const QString &file_name, const QString &to_uin, const QString &msg_id);
    void loadGroupChatImg(const QString &file_name, QString uin,
                          const QString &gcode, QString fid, QString rip,
                          QString rport, const QString &time);

protected:
    void run();

    void saveImg(const QByteArray &array, QString path);
    QByteArray requestImgData(QString host, QString request_url);
    QString getHost(const QByteArray &url) const;
    QString getRequestUrl(const QByteArray &url) const;

    virtual QByteArray getImgUrl(const LoadInfo &info) const;

protected:
    QQueue<LoadInfo> infos_;

private:
    QMutex lock_;
    bool has_error_;
};

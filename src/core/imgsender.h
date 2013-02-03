#ifndef IMGSENDER_H
#define IMGSENDER_H

#include <QThread>
#include <QMap>

#include "types.h"
#include "request.h"
#include "core/types.h"

//快速上传两次图片可能会丢失，记得更改
class ImgSender : public QThread
{
    Q_OBJECT
signals:
    void postResult(const QString, FileInfo file_info);
    void sendDone(const QString&, const QString&);

public:
    ImgSender() : has_error_(false), is_sending_(false) {}
    ~ImgSender() {}

public:
    void send(const QString uinque_id, const QString full_path);

    bool hasError()
    { return has_error_; }
    bool isSendding()
    { return is_sending_; }

    bool getFileInfo(const QString &id, FileInfo &info)
    {
        if ( file_infos_.contains(id) )
        {
            info.size = file_infos_[id].size;
            info.network_path = file_infos_[id].network_path;
            info.name = file_infos_[id].name;
            return true;
        }

        return false;
    }

protected:
    void run();

protected:
    QString base_send_url_;
    QString full_path_;
    QString host_;

private:
    virtual QByteArray createSendMsg(const QByteArray &file_data, const QString &boundary) = 0;
    virtual FileInfo parseResult(const QByteArray &array) = 0;

private:
    Request req_;
    QByteArray msg;
    bool has_error_;
    QString unique_id_;
    bool is_sending_;

    QMap<QString, FileInfo> file_infos_;
};

#endif //IMGSENDER_H

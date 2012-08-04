#pragma once

#include <QThread>

#include "types.h"
#include "request.h"

//快速上传两次图片可能会丢失，记得更改
class ImgSender : public QThread
{
    Q_OBJECT
public:
    ImgSender() : has_error_(false), is_sending_(false) {}
    ~ImgSender() {}

public:
    void send(const QString uinque_id, const QString full_path);

    bool hasError()
    { return has_error_; }
    bool isSendding()
    { return is_sending_; }

signals:
    void postResult(const QString, FileInfo file_info);
    void sendDone(const QString&, const QString&);

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
};

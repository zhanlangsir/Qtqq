#pragma once

#include "types.h"
#include "request.h"
#include "imgsender.h"

class GroupImgSender : public ImgSender
{
public:
    GroupImgSender();

private:
    virtual QByteArray createSendMsg(const QByteArray &file_data, const QString &boundary);
    FileInfo parseResult(const QByteArray &array);
};

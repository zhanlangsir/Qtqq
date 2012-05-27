#ifndef QTQQ_FRIENDIMGSENDER_H
#define QTQQ_FRIENDIMGSENDER_H

#include "types.h"
#include "request.h"
#include "imgsender.h"

class FriendImgSender : public ImgSender
{
public:
    FriendImgSender();

private:
    virtual QByteArray createSendMsg(const QByteArray &file_data, const QString &boundary);
    FileInfo parseResult(const QByteArray &array);
};

#endif //QTQQ_FRIENDIMGSENDER_H

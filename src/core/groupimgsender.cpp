#include "groupimgsender.h"

#include <QByteArray>
#include <QFileInfo>
#include <QTcpSocket>
#include <QDebug>
#include <QByteArray>

#include "captchainfo.h"
#include "request.h"

GroupImgSender::GroupImgSender()
{
    base_send_url_ = "/cgi-bin/cface_upload?time=";
    host_ = "up.web2.qq.com";
}

QByteArray GroupImgSender::createSendMsg(const QByteArray &file_data, const QString &boundary)
{
    QByteArray boundary_convenience ="--" + boundary.toAscii() + "\r\n";

    QByteArray msg = boundary_convenience + "Content-Disposition: form-data; name=\"from\"\r\n\r\n"
    "control\r\n"+boundary_convenience+"Content-Disposition: form-data; name=\"f\"\r\n\r\n"
    "EQQ.Model.ChatMsg.callbackSendPicGroup\r\n"+boundary_convenience+"Content-Disposition: form-data; name=\"vfwebqq\"\r\n\r\n" +
    CaptchaInfo::instance()->vfwebqq().toAscii()+ "\r\n"+boundary_convenience+"Content-Disposition: form-data; name=\"custom_face\"; filename=\""+ QFileInfo(full_path_).fileName().toAscii() + "\"\r\n"
    "Content-Type: image/jpeg\r\n\r\n" + file_data +"\r\n"+boundary_convenience+"Content-Disposition: form-data; name=\"fileid\"\r\n\r\n"
    "1\r\n--"+boundary.toAscii() +"--\r\n\r\n";

    return msg;
}

FileInfo GroupImgSender::parseResult(const QByteArray &array)
{
    int ret_idx = array.indexOf("'ret':")+6;
    int ret_end_idx = array.indexOf(",", ret_idx);
    int ret = array.mid(ret_idx, ret_end_idx - ret_idx).toInt();

    int file_name_idx = array.indexOf("'msg':")+7;
    int file_name_end_idx;

    if (ret == 4)
         file_name_end_idx = array.indexOf(" ", file_name_idx+1);
    else
        file_name_end_idx = array.indexOf("'", file_name_idx+1);

    QString file_name = array.mid(file_name_idx, file_name_end_idx - file_name_idx);

    FileInfo file_info = {0, file_name , ""};
    return file_info;
}

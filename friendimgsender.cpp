#include "friendimgsender.h"
#include "request.h"

#include <QByteArray>
#include <QFileInfo>
#include <QTcpSocket>
#include <QDateTime>
#include <QDebug>

FriendImgSender::FriendImgSender()
{
    base_send_url_ = "/ftn_access/upload_offline_pic?time=";
    host_ = "weboffline.ftn.qq.com";
}

QByteArray FriendImgSender::createSendMsg(const QByteArray &file_data, const QString &boundary)
{
    QByteArray boundary_convenience ="--" + boundary.toAscii() + "\r\n";

    QByteArray msg = boundary_convenience + "Content-Disposition: form-data; name=\"callback\"\r\n\r\n"
        "parent.EQQ.Model.ChatMsg.callbackSendPic\r\n"+boundary_convenience+"Content-Disposition: form-data; name=\"locallangid\"\r\n\r\n"
        "2052\r\n"+boundary_convenience+"Content-Disposition: form-data; name=\"clientversion\"\r\n\r\n"
        "1409\r\n"+boundary_convenience+"Content-Disposition: form-data; name=\"uin\"\r\n\r\n" + id_.toAscii() + "\r\n"
        +boundary_convenience+"Content-Disposition: form-data; name=\"skey\"\r\n\r\n" + cap_info_.skey_.toAscii() + "\r\n"
        +boundary_convenience+"Content-Disposition: form-data; name=\"appid\"\r\n\r\n"
        "1002101\r\n"+boundary_convenience+"Content-Disposition: form-data; name=\"peeruin\"\r\n\r\n"
        "593023668\r\n"+boundary_convenience+"Content-Disposition: form-data; name=\"file\"; filename=\""+ QFileInfo(full_path_).fileName().toAscii() + "\"\r\n"
        "Content-Type: image/jpeg\r\n\r\n" + file_data +"\r\n"+boundary_convenience+"Content-Disposition: form-data; name=\"fileid\"\r\n\r\n"
        "1\r\n"+boundary_convenience+"Content-Disposition: form-data; name=\"vfwebqq\"\r\n\r\n" + cap_info_.vfwebqq_.toAscii() + "\r\n"
        +boundary_convenience+"Content-Disposition: form-data; name=\"senderviplevel\"\r\n\r\n"
        "0\r\n"+boundary_convenience+"Content-Disposition: form-data; name=\"reciverviplevel\"\r\n\r\n"
        "0\r\n--"+boundary.toAscii() +"--\r\n\r\n";

    return msg;
}

FileInfo FriendImgSender::parseResult(const QByteArray &array)
{
    int file_size_idx = array.indexOf("filesize")+10;
    int file_size_end_idx = array.indexOf(",", file_size_idx);
    int file_name_idx = array.indexOf("filename")+11;
    int file_name_end_idx = array.indexOf("\"", file_name_idx);
    int network_path_idx = array.indexOf("filepath")+11;
    int network_path_end_idx = array.indexOf("\"", network_path_idx);

    int file_size = array.mid(file_size_idx, file_size_end_idx - file_size_idx).toInt();
    QString file_name = array.mid(file_name_idx, file_name_end_idx - file_name_idx);
    QString network_path = array.mid(network_path_idx, network_path_end_idx - network_path_idx);


    QTcpSocket fd;
    QString apply_offline_pic_url = "/channel/apply_offline_pic_dl2?f_uin=" +
        id_ + "&file_path=" + network_path + "&clientid=5412354841&psessionid="+cap_info_.psessionid_ + "&t=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());

    Request req;
    req.create(kGet, apply_offline_pic_url);
    req.addHeaderItem("Host", "d.web2.qq.com");
    req.addHeaderItem("Connection", "keep-alive");
    req.addHeaderItem("Content-Type", "utf-8");
    req.addHeaderItem("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002");
    req.addHeaderItem("Cookie", cap_info_.cookie_);

    fd.connectToHost("d.web2.qq.com", 80);
    fd.write(req.toByteArray());

    fd.close();
    FileInfo file_info = {file_size, file_name, network_path};
    return file_info;
}


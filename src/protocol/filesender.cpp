#include "filesender.h"

#include <QTcpSocket>
#include <QDateTime>
#include <QFileInfo>

#include "core/captchainfo.h"
#include "core/talkable.h"
#include "core/curr_login_account.h"
#include "core/request.h"

Protocol::FileSender::FileSender()
{
    boundary_ = "----WebKitFormBoundaryk5nH7APtIbShxvqE";// + QString(QByteArray::number(QDateTime::currentDateTime().toMSecsSinceEpoch()).toHex()).toAscii();
}

QByteArray Protocol::FileSender::createFileData(const QString &file_path, const QByteArray &file_data)
{
    QByteArray boundary = "--"+boundary_;

    QString file_name = QFileInfo(file_path).fileName();
    QByteArray msg = boundary + "\r\n" + 
    "Content-Disposition: form-data; name=\"file\"; filename=\"" + file_name.toAscii() + "\"\r\n" +
    "Content-Type: application/octet-stream\r\n\r\n" + file_data + "\r\n" + boundary + "--\r\n";

    return msg;
}

QByteArray Protocol::FileSender::createOffFileData(const QString &file_path, const QString &to_id, const QByteArray &file_data)
{
    QByteArray boundary = "--"+boundary_+"\r\n";
    QByteArray msg = boundary + 
        "Content-Disposition: form-data; name=\"callback\"\r\n\r\n"
        "parent.EQQ.Model.ChatMsg.callbackSendOffFile\r\n"+
        boundary+
        "Content-Disposition: form-data; name=\"locallangid\"\r\n\r\n2052\r\n"+
        boundary+
        "Content-Disposition: form-data; name=\"clientversion\"\r\n\r\n1409\r\n"+
        boundary+
        "Content-Disposition: form-data; name=\"uin\"\r\n\r\n" + CurrLoginAccount::id().toAscii() + "\r\n" +
        boundary+
        "Content-Disposition: form-data; name=\"skey\"\r\n\r\n" + CaptchaInfo::instance()->skey().toAscii() + "\r\n" +
        boundary+
        "Content-Disposition: form-data; name=\"appid\"\r\n\r\n1002101\r\n"+
        boundary+
        "Content-Disposition: form-data; name=\"peeruin\"\r\n\r\n" + to_id.toAscii() + "\r\n"+
        boundary+
        "Content-Disposition: form-data; name=\"vfwebqq\"\r\n\r\n" + CaptchaInfo::instance()->vfwebqq().toAscii() + "\r\n" +
        boundary +
        "Content-Disposition: form-data; name=\"file\"; filename=\"" + QFileInfo(file_path).fileName().toAscii() + "\"\r\n"
        "Content-Type: application/octet-stream\r\n\r\n" + file_data +"\r\n"+
        boundary+
        "Content-Disposition: form-data; name=\"fileid\"\r\n\r\n1\r\n"+
        boundary+
        "Content-Disposition: form-data; name=\"senderviplevel\"\r\n\r\n0\r\n"+
        boundary+
        "Content-Disposition: form-data; name=\"reciverviplevel\"\r\n\r\n0\r\n" +
        "--"+boundary_+"--\r\n";

    return msg;
}

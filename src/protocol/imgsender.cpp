#include "imgsender.h"

#include <QTcpSocket>
#include <QDateTime>
#include <QFileInfo>

#include "core/captchainfo.h"
#include "core/talkable.h"
#include "core/curr_login_account.h"
#include "core/request.h"

Protocol::ImgSender::ImgSender()
{
    boundary_ = "----WebKitFormBoundaryk5nH7APtIbShxvqE";// + QString(QByteArray::number(QDateTime::currentDateTime().toMSecsSinceEpoch()).toHex()).toAscii();
    getKeyAndSig();
}

QByteArray Protocol::ImgSender::createOffpicBody(const QString &file_path, const QByteArray &file_data)
{
    QByteArray boundary_convenience ="--" + boundary_.toAscii() + "\r\n";

    QByteArray msg = boundary_convenience + 
        "Content-Disposition: form-data; name=\"callback\"\r\n\r\n"
        "parent.EQQ.Model.ChatMsg.callbackSendPic\r\n"+
        boundary_convenience+
        "Content-Disposition: form-data; name=\"locallangid\"\r\n\r\n2052\r\n"+
        boundary_convenience+
        "Content-Disposition: form-data; name=\"clientversion\"\r\n\r\n1409\r\n"+
        boundary_convenience+
        "Content-Disposition: form-data; name=\"uin\"\r\n\r\n" + CurrLoginAccount::id().toAscii() + "\r\n"+
        boundary_convenience+
        "Content-Disposition: form-data; name=\"skey\"\r\n\r\n" + CaptchaInfo::instance()->skey().toAscii() + "\r\n"+
        boundary_convenience+
        "Content-Disposition: form-data; name=\"appid\"\r\n\r\n1002101\r\n"+
        boundary_convenience+
        "Content-Disposition: form-data; name=\"peeruin\"\r\n\r\n593023668\r\n"+
        boundary_convenience+
        "Content-Disposition: form-data; name=\"file\"; filename=\"" + QFileInfo(file_path).fileName().toAscii() + "\"\r\n"
        "Content-Type: image/jpeg\r\n\r\n" + file_data +"\r\n"+
        boundary_convenience+
        "Content-Disposition: form-data; name=\"fileid\"\r\n\r\n1\r\n"+
        boundary_convenience+
        "Content-Disposition: form-data; name=\"vfwebqq\"\r\n\r\n" + CaptchaInfo::instance()->vfwebqq().toAscii() + "\r\n"+
        boundary_convenience+
        "Content-Disposition: form-data; name=\"senderviplevel\"\r\n\r\n0\r\n"+
        boundary_convenience+
        "Content-Disposition: form-data; name=\"reciverviplevel\"\r\n\r\n0\r\n" + 
        "--" + boundary_.toAscii() +"--\r\n\r\n";

    return msg;
}

QByteArray Protocol::ImgSender::createGroupImgBody(const QString &file_path, const QByteArray &file_data)
{
    QByteArray boundary_convenience ="--" + boundary_.toAscii() + "\r\n";

    QByteArray msg = boundary_convenience + 
        "Content-Disposition: form-data; name=\"from\"\r\n\r\ncontrol\r\n"+
        boundary_convenience+
        "Content-Disposition: form-data; name=\"f\"\r\n\r\nEQQ.Model.ChatMsg.callbackSendPicGroup\r\n"+
        boundary_convenience+
        "Content-Disposition: form-data; name=\"vfwebqq\"\r\n\r\n" + CaptchaInfo::instance()->vfwebqq().toAscii()+ "\r\n"+
        boundary_convenience+
        "Content-Disposition: form-data; name=\"custom_face\"; filename=\""+ QFileInfo(file_path).fileName().toAscii() + "\"\r\n"
        "Content-Type: image/jpeg\r\n\r\n" + file_data +"\r\n"+
        boundary_convenience+
        "Content-Disposition: form-data; name=\"fileid\"\r\n\r\n1\r\n" + 
        "--"+boundary_.toAscii() +"--\r\n\r\n";

    return msg;
}

bool Protocol::ImgSender::parseMsgResult(QString file_path, const QByteArray &array)
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
        CurrLoginAccount::id() + "&file_path=" + network_path + "&clientid=5412354841&psessionid="+CaptchaInfo::instance()->psessionid() +
            "&t=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch());

    Request req;
    req.create(kGet, apply_offline_pic_url);
    req.addHeaderItem("Host", "d.web2.qq.com");
    req.addHeaderItem("Connection", "keep-alive");
    req.addHeaderItem("Content-Type", "utf-8");
    req.addHeaderItem("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002");
    req.addHeaderItem("Cookie", CaptchaInfo::instance()->cookie());

    fd.connectToHost("d.web2.qq.com", 80);
    fd.write(req.toByteArray());
    fd.close();

    FileInfo file_info;
    file_info.size =file_size;
    file_info.name = file_name;
    file_info.network_path = network_path;

    sended_imgs_.insert(file_path, file_info);

    return true;
}


bool Protocol::ImgSender::parseGroupMsgResult(QString file_path, const QByteArray &array)
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

    sended_imgs_.insert(file_path, file_info);

    return true;
}

void Protocol::ImgSender::getKeyAndSig()
{
    QString gface_sig_url = "/channel/get_gface_sig2?clientid=5412354841&psessionid="+CaptchaInfo::instance()->psessionid() +
        "&t="+QString::number(QDateTime::currentMSecsSinceEpoch());

    QHttpRequestHeader header;
    header.setRequest("GET", gface_sig_url);
    header.addValue("Host", "d.web2.qq.com");
    header.addValue("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002");
    header.addValue("Cookie", CaptchaInfo::instance()->cookie());

    http_.setHost("d.web2.qq.com");
    connect(&http_, SIGNAL(done(bool)), this, SLOT(getKeyAndSigDone(bool)));

    http_.request(header);
}

void Protocol::ImgSender::getKeyAndSigDone(bool err)
{
    QByteArray array = http_.readAll();

    int gface_key_idx = array.indexOf("gface_key")+12;
    int gface_key_end_idx = array.indexOf(",",gface_key_idx)-1;

    int gface_sig_idx = array.indexOf("gface_sig")+12;
    int gface_sig_end_idx = array.indexOf("}", gface_sig_idx)-1;

    key_ = array.mid(gface_key_idx, gface_key_end_idx - gface_key_idx);
    sig_ = array.mid(gface_sig_idx, gface_sig_end_idx - gface_sig_idx);

    qDebug() << "Get key and sig done!\n" 
             << "key: " << key_
             << "sig: " << sig_ 
             << endl;
}

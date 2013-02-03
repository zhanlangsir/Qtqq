#include "msgsender.h"

#include <QTcpSocket>
#include <QDebug>
/*

void MsgSender::send(const Request &req)
{
    msgs_be_send_.enqueue(req);
    start();
}

void MsgSender::run()
{  
    while (msgs_be_send_.count() != 0)
    {
        Request req = msgs_be_send_.dequeue();

        QTcpSocket fd;
        fd.connectToHost("d.web2.qq.com", 80);
        fd.write(req.toByteArray());

        fd.waitForReadyRead();
        qDebug()<<fd.readAll()<<endl;

        emit sendDone(true, "msg send done");

        fd.close();
    }
}

QString MsgSender::groupMsgToJson(const QVector<QQChatItem> &items)
{
    bool has_gface = false;
    QString json_msg;

    foreach ( const QQChatItem &item, items )
    {
        switch ( item.type() )
        {
            case QQChatItem::kWord:
                json_msg.append("\\\"" + item.content() + "\\\",");
                break;
            case QQChatItem::kQQFace:
                json_msg.append("[\\\"face\\\"," + item.content() + "],");
                break;
            case QQChatItem::kGroupChatImg:
                has_gface = false;
                json_msg.append("[\\\"cface\\\",\\\"group\\\",\\\"" + getUploadedFileInfo(item.content()).name + "\\\"],");
                break;
        }
    }

    json_msg = json_msg +
        "[\\\"font\\\",{\\\"name\\\":\\\"%E5%AE%8B%E4%BD%93\\\",\\\"size\\\":\\\"10\\\",\\\"style\\\":[0,0,0],\\\"color\\\":\\\"000000\\\"}]]\","
        "\"msg_id\":" + QString::number(msg_id_++) + ",\"clientid\":\"5412354841\","
        "\"psessionid\":\""+ CaptchaInfo::instance()->psessionid() +"\"}"
        "&clientid=5412354841&psessionid="+CaptchaInfo::instance()->psessionid();

    if (has_gface)
    {
        ImgSender *sender = Protocol::QQProtocol::instance()->groupImgSender();
        json_msg = "r={\"group_uin\":" + id() +",\"group_code\":" + sender->code() + "," + "\"key\":\"" + key()+ "\"," +
            "\"sig\":\"" + sender->sig() + "\", \"content\":\"[" + json_msg;
    }
    else
        json_msg = "r={\"group_uin\":" + id() + ",\"content\":\"[" + json_msg;


    return json_msg;
}

QString MsgSender::msgToJson(const QVector<QQChatItem> &items)
{
    QString json_msg = "r={\"to\":" + id() +",\"face\":525,\"content\":\"[";

    foreach ( const QQChatItem &item, items )
    {
        switch ( item.type() )
        {
            case QQChatItem::kWord:
                json_msg.append("\\\"" + item.content() + "\\\",");
                break;
            case QQChatItem::kQQFace:
                json_msg.append("[\\\"face\\\"," + item.content() + "],");
                break;
            case QQChatItem::kFriendOffpic:
                json_msg.append("[\\\"offpic\\\",\\\"" + getUploadedFileInfo(item.content()).network_path + "\\\",\\\"" + getUploadedFileInfo(item.content()).name + "\\\"," + QString::number(getUploadedFileInfo(item.content()).size) + "],");
                break;
        }
    }

    json_msg = json_msg +
        "[\\\"font\\\",{\\\"name\\\":\\\"%E5%AE%8B%E4%BD%93\\\",\\\"size\\\":\\\"10\\\",\\\"style\\\":[0,0,0],\\\"color\\\":\\\"000000\\\"}]]\","
        "\"msg_id\":" + QString::number(msg_id_++) + ",\"clientid\":\"5412354841\","
        "\"psessionid\":\""+ CaptchaInfo::instance()->psessionid() +"\"}"
        "&clientid=5412354841&psessionid="+CaptchaInfo::instance()->psessionid();
    return json_msg;
}
*/

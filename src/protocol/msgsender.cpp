#include "msgsender.h"

#include <QTcpSocket>
#include <QDateTime>
#include <QDebug>

#include "core/talkable.h"
#include "protocol/qq_protocol.h"
#include "core/captchainfo.h"
#include "protocol/imgsender.h"
#include "core/sockethelper.h"

int Protocol::MsgSender::msg_id_ = 4462000;  //arbitrary

QString Protocol::MsgSender::msgToJson(Contact *to, const QVector<QQChatItem> &items)
{
    QString json_msg = "r={\"to\":" + to->id() +",\"face\":525,\"content\":\"[";

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
                {
                    Protocol::ImgSender *imgsender = Protocol::QQProtocol::instance()->imgSender();
                    FileInfo info = imgsender->getUploadedImgInfo(item.content());
                    json_msg.append("[\\\"offpic\\\",\\\"" + info.network_path + "\\\",\\\"" + info.name + "\\\"," + QString::number(info.size) + "],");
                }
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

QString Protocol::MsgSender::groupMsgToJson(Group *to, const QVector<QQChatItem> &items)
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
                {
                    has_gface = true;
                    Protocol::ImgSender *imgsender = Protocol::QQProtocol::instance()->imgSender();

                    FileInfo info = imgsender->getUploadedImgInfo(item.content());
                    json_msg.append("[\\\"cface\\\",\\\"group\\\",\\\"" + info.name + "\\\"],");

                }
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
        Protocol::ImgSender *sender = Protocol::QQProtocol::instance()->imgSender();
        json_msg = "r={\"group_uin\":" + to->id() +",\"group_code\":" + to->gcode() + "," + "\"key\":\"" + sender->key()+ "\"," +
            "\"sig\":\"" + sender->sig() + "\", \"content\":\"[" + json_msg;
    }
    else
        json_msg = "r={\"group_uin\":" + to->id() + ",\"content\":\"[" + json_msg;

    return json_msg;
}

QString Protocol::MsgSender::sessMsgToJson(Contact *to, Group *group, const QVector<QQChatItem> &msgs)
{
    QString json_msg = "r={\"to\":" + to->id() + ",\"group_sig\":\"" + getGroupSig(group->id(), to->id()) + "\",\"face\":291,\"content\":\"[";

    foreach ( const QQChatItem &item, msgs )
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
                //陌生人不支持图片发送
                //json_msg.append("[\\\"offpic\\\",\\\"" + getUploadedFileInfo(item.content()).network_path + "\\\",\\\"" + getUploadedFileInfo(item.content()).name + "\\\"," + QString::number(getUploadedFileInfo(item.content()).size) + "],");
                break;
        }
    }
    json_msg = json_msg +
        "[\\\"font\\\",{\\\"name\\\":\\\"%E5%AE%8B%E4%BD%93\\\",\\\"size\\\":\\\"10\\\",\\\"style\\\":[0,0,0],\\\"color\\\":\\\"000000\\\"}]]\","
        "\"msg_id\":" + QString::number(msg_id_++) + ",\"service_type\":0,\"clientid\":\"5412354841\","
        "\"psessionid\":\""+ CaptchaInfo::instance()->psessionid() +"\"}"
        "&clientid=5412354841&psessionid="+CaptchaInfo::instance()->psessionid();

    return json_msg;
}

QString Protocol::MsgSender::getGroupSig(QString gid, QString to_id)
{
    if ( group_sigs_.contains(gid+to_id) )
        return group_sigs_.value(gid+to_id);

    QString msg_sig_url = "/channel/get_c2cmsg_sig2?id="+ gid +"&to_uin=" + to_id +
        "&service_type=0&clientid=5412354841&psessionid=" + CaptchaInfo::instance()->psessionid() +"&t=" + QString::number(QDateTime::currentMSecsSinceEpoch());

    QHttpRequestHeader header;
    QString host = "d.web2.qq.com";
    header.setRequest("GET", msg_sig_url);
    header.addValue("Host", host);
    header.addValue("Content-Type", "utf-8");
    header.addValue("Referer", "http://d.web2.qq.com/proxy.html?v=20110331002");
    header.addValue("Cookie", CaptchaInfo::instance()->cookie());

    QTcpSocket fd;
    fd.connectToHost(host, 80);
    fd.write(header.toString().toAscii());

    QByteArray result;
    socketReceive(&fd, result);
    fd.close();

    int sig_s_idx = result.indexOf("value")+8;
    int sig_e_idx = result.indexOf('"', sig_s_idx);
    QString sig = result.mid(sig_s_idx, sig_e_idx - sig_s_idx);

    group_sigs_.insert(gid+to_id, sig);

    return sig;
}

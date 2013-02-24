#include "msgsender.h"

#include <QDebug>

#include "core/talkable.h"
#include "protocol/qq_protocol.h"
#include "core/captchainfo.h"
#include "protocol/imgsender.h"

int MsgSender::msg_id_ = 4462000;  //arbitrary

QString MsgSender::msgToJson(Contact *to, const QVector<QQChatItem> &items)
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

QString MsgSender::groupMsgToJson(Group *to, const QVector<QQChatItem> &items)
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

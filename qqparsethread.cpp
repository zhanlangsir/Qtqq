#include <QDebug>
#include "qqparsethread.h"
#include "include/json/json.h"
#include "qqutility.h"

void QQParseThread::pushRawMsg(QByteArray msg)
{
    lock_.lock();
    message_queue_.enqueue(msg);   
    lock_.unlock();

    start();
}

void QQParseThread::run()
{
    while (true)
    {
        lock_.lock();
        int rawmsg_count = message_queue_.count();
        lock_.unlock();
        
        if (rawmsg_count == 0)
            break;
        
        Json::Reader reader;
        Json::Value root;

        QByteArray unparse_msg = message_queue_.dequeue();
        qDebug()<<unparse_msg<<endl;
        int idx = unparse_msg.indexOf("\r\n\r\n");
        unparse_msg = unparse_msg.mid(idx+4);

        if (!reader.parse(QString(unparse_msg).toStdString(), root, false))
        {
            continue;
        }    

        if (root["retcode"].asInt() == 121)
        {
            //ÀëÏß
        }
        for (unsigned int i = 0; i < root["result"].size(); ++i)
        {
            const Json::Value result = root["result"][i];
            QString type = QString::fromStdString(result["poll_type"].asString());
            ShareQQMsgPtr msg(createMsg(type, result));
            if (msg)
                emit parseDone(msg);
        }
    }
}

QQMsg* QQParseThread::createMsg(QString type, const Json::Value result)
{
    if (type ==  "group_message")
    {
        QQGroupChatMsg *g_chat_msg = new QQGroupChatMsg();
        g_chat_msg->set_type(QQMsg::kGroup);
        g_chat_msg->send_uin_ = QString::number(result["value"]["send_uin"].asLargestInt());
        g_chat_msg->from_uin_ = QString::number(result["value"]["from_uin"].asLargestInt());
        g_chat_msg->to_uin_ = QString::number(result["value"]["from_uin"].asLargestInt());
        g_chat_msg->info_seq_ = QString::number(result["value"]["info_seq"].asLargestInt());
        g_chat_msg->group_code_ =QString::number(result["value"]["group_code"].asLargestInt());
        g_chat_msg->color_ = QString::fromStdString(result["value"]["content"][0][1]["color"].asString());
        g_chat_msg->size_ = result["value"]["content"][0][1]["size"].asInt();
        g_chat_msg->time_ = result["value"]["time"].asLargestInt();
        g_chat_msg->font_name_ = QString::fromStdString(result["value"]["content"][0][1]["name"].asString());

        for (unsigned int i = 1; i < result["value"]["content"].size(); ++i)
        {
            QQChatItem item;
            Json::Value content = result["value"]["content"][i];

            if (content.type() == Json::stringValue)
            {
                item.set_type(QQChatItem::kWord);
                item.set_content(QString::fromStdString(content.asString()));
            }
            else
            {
                QString face_type =  QString::fromStdString(content[0].asString());

                if (face_type == "face")
                {
                    item.set_type(QQChatItem::kQQFace);
                    item.set_content(QString::number(content[1].asInt()));
                }
                else
                {
                    item.set_type(QQChatItem::kGroupChatImg);
                    item.set_content(QString::fromStdString(content[1]["name"].asString()));
                    item.set_file_id(QString::number(content[1]["file_id"].asLargestInt()));
                    QString server = QString::fromStdString(content[1]["server"].asString());
                    int ip_end_idx = server.indexOf(":");
                    item.set_server_ip(server.mid(0, ip_end_idx));
                    item.set_server_port(server.mid(ip_end_idx+1));
                }
            }
            g_chat_msg->msg_.append(item);
        }
        return g_chat_msg;
    }
    else if (type == "buddies_status_change")
    {
        QQStatusChangeMsg *status_msg = new QQStatusChangeMsg();
        status_msg->set_type(QQMsg::kBuddiesStatusChange);
        status_msg->uin_ = QString::number(result["value"]["uin"].asLargestInt());
        status_msg->client_type_ = (ClientType)result["value"]["client_type"].asInt();
        QString status_str = QString::fromStdString(result["value"]["status"].asString());
        status_msg->status_ = QQUtility::stringToStatus(status_str);

        return status_msg;
    }
    else if (type == "sys_g_msg")
    {
        QQSystemGMsg *system_g_msg = new QQSystemGMsg();
        system_g_msg->set_type(QQMsg::kSystemG);
        system_g_msg->msg_id_ = QString::number(result["value"]["msg_id"].asInt());
        system_g_msg->from_uin_ = QString::number(result["value"]["from_uin"].asLargestInt());
        system_g_msg->to_uin_ = QString::number(result["value"]["from_uin"].asLargestInt());
        system_g_msg->msg_id2_ = QString::number(result["value"]["msg_id2"].asInt());
        system_g_msg->sys_g_type_ =QString::fromStdString(result["value"]["type"].asString());
        system_g_msg->gcode_ = QString::number(result["value"]["gcode"].asLargestInt());
        system_g_msg->t_gcode_ = QString::number(result["value"]["t_gcode"].asLargestInt());
        system_g_msg->request_uin_ = QString::number(result["value"]["request_uin"].asLargestInt());
        system_g_msg->t_request_uin_ = QString::fromStdString(result["value"]["t_request_uin"].asString());
        system_g_msg->msg_ = QString::fromStdString(result["value"]["msg"].asString());
        return system_g_msg;
    }
    else if (type == "message")
    {
        QQChatMsg *chat_msg = new QQChatMsg();
        chat_msg->set_type(QQMsg::kFriend);
        chat_msg->from_uin_ = QString::number(result["value"]["from_uin"].asLargestInt());
        chat_msg->msg_id_ = QString::number(result["value"]["msg_id"].asInt());
        chat_msg->to_uin_ = QString::number(result["value"]["to_uin"].asLargestInt());
        chat_msg->color_ = QString::fromStdString(result["value"]["content"][0][1]["color"].asString());
        chat_msg->size_ = result["value"]["content"][0][1]["size"].asInt();
        chat_msg->time_ = result["value"]["time"].asLargestInt();
        chat_msg->font_name_ = QString::fromStdString(result["value"]["content"][0][1]["name"].asString());

        for (unsigned int i = 1; i < result["value"]["content"].size(); ++i)
        {
            QQChatItem item;
            Json::Value content = result["value"]["content"][i];

            if (content.type() == Json::stringValue)
            {
                item.set_type(QQChatItem::kWord);
                item.set_content(QString::fromStdString(content.asString()));
            }
            else
            {
                QString face_type =  QString::fromStdString(content[0].asString());

                if (face_type == "face")
                {
                    item.set_type(QQChatItem::kQQFace);
                    item.set_content(QString::number(content[1].asInt()));
                }
                else if (face_type == "offpic")
                {
                    item.set_type(QQChatItem::kFriendOffpic);
                    item.set_content(QString::fromStdString(content[1]["file_path"].asString()));
                }
                else
                {
                    item.set_type(QQChatItem::kFriendCface);
                    item.set_content(QString::fromStdString(content[1].asString()));
                }
            }
            chat_msg->msg_.append(item);
        }

        return chat_msg;
    }
    else if (type == "system_message")
    {
        QQSystemMsg *system_msg = new QQSystemMsg();
        system_msg->set_type(QQMsg::kSystem);
        system_msg->systemmsg_type_ = QString::fromStdString(result["value"]["type"].asString());
        system_msg->from_ = QString::number(result["value"]["from_uin"].asLargestInt());
        system_msg->account_ = QString::number(result["value"]["account"].asLargestInt());
        system_msg->msg_ = QString::fromStdString(result["value"]["msg"].asString());
        system_msg->status_ = (FriendStatus)(result["value"]["stat"].asInt());

        return system_msg;
    }
    else
    {
        qDebug()<<"unknow type"<<endl;
        return NULL;
    }
}

QQParseThread::QQParseThread()
{
    qRegisterMetaType<ShareQQMsgPtr>("ShareQQMsgPtr");
}

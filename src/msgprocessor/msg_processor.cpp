#include "msg_processor.h"

#include <QDebug>

#include "core/qqutility.h"

#define FRIEND_MSG_TYPE "message"
#define GROUP_MSG_TYPE "group_message"
#define SESS_MSG_TYPE "sess_message"
#define STATUS_CHANGE_TYPE "buddies_status_change"
#define SYS_G_MSG_TYPE "sys_g_msg"
#define SYS_MSG_TYPE "system_message"
#define FILE_TYPE "file_message"
#define OFFFILE_TYPE "push_offfile"
#define FILESRV_TRANSFER_TYPE "filesrv_transfer"

MsgProcessor *MsgProcessor::instance_ = NULL;

MsgProcessor::MsgProcessor()
{
	setTerminationEnabled(true);
	qRegisterMetaType<ShareQQMsgPtr>("ShareQQMsgPtr");

	connect(this, SIGNAL(newFriendChatMsg(ShareQQMsgPtr)), this, SIGNAL(newChatMsg(ShareQQMsgPtr)));
	connect(this, SIGNAL(newGroupChatMsg(ShareQQMsgPtr)), this, SIGNAL(newChatMsg(ShareQQMsgPtr)));
	connect(this, SIGNAL(newSessChatMsg(ShareQQMsgPtr)), this, SIGNAL(newChatMsg(ShareQQMsgPtr)));
}

MsgProcessor::~MsgProcessor()
{
	stop();
}

void MsgProcessor::pushRawMsg(QByteArray msg)
{
    lock_.lock();
    message_queue_.enqueue(msg);   
    lock_.unlock();

    start();
}

void MsgProcessor::run()
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

		int idx = unparse_msg.indexOf("\r\n\r\n");
		unparse_msg = unparse_msg.mid(idx+4);

		if (!reader.parse(QString(unparse_msg).toStdString(), root, false))
		{
			continue;
		}    

		qDebug() << "Recive msg:\n" 
            << QString::fromStdString(root.toStyledString()) << endl;

		if (root["retcode"].asInt() == 121)
		{

		}

		QVector<QQMsg *> be_sorting_msg;
		for (unsigned int i = 0; i < root["result"].size(); ++i)
		{
			const Json::Value result = root["result"][i];

			if ( isMsgRepeat(result) )
				continue;

			QString type = QString::fromStdString(result["poll_type"].asString());

			QQMsg *msg = createMsg(type, result);

			if ( msg )
				be_sorting_msg.append(msg);
		}

		if ( be_sorting_msg.size() > 1 )
			sortByTime(be_sorting_msg);

		dispatchMsg(be_sorting_msg);		
	}
}

bool MsgProcessor::isMsgRepeat(const Json::Value &msg)
{
	if ( pre_msg_ == msg )
	{
		qDebug()<<"get rid of confire msg :"<<QString::fromStdString(pre_msg_.toStyledString())<<endl;
		return true;
	}

	pre_msg_ = msg;
	return false;
}

void MsgProcessor::sortByTime(QVector<QQMsg*> &be_sorting_msg) const
{
	//插入排序,按时间从小到大排序
	for ( int i = 1; i < be_sorting_msg.size(); ++i )
	{
		QQMsg *msg = be_sorting_msg[i];

		int j = i - 1;
		while ( j > 0 && be_sorting_msg[j]->time() > msg->time() )
		{
			be_sorting_msg[j + 1] = be_sorting_msg[j];
			--j;
		}

		be_sorting_msg[j+1] = msg;
	} 
}

QQMsg* MsgProcessor::createMsg(QString type, const Json::Value result)
{
	if ( type == FRIEND_MSG_TYPE )
    {
        return createFriendMsg(result); 
    }
    else if ( type == GROUP_MSG_TYPE ) 
    {
        return createGroupMsg(result);
    }
    else if ( type == SESS_MSG_TYPE )
    {
        return createSessMsg(result); 
    }
    else if ( type == STATUS_CHANGE_TYPE ) 
    {
        return createBuddiesStatusChangeMsg(result); 
    }
    else if ( type == SYS_G_MSG_TYPE ) 
    {
        return createSystemGroupMsg(result); 
    }
    else if ( type == SYS_MSG_TYPE ) 
    {
        return createSystemMsg(result); 
    }
    else if ( type == FILE_TYPE ) 
    {
        return createFileMsg(result);
    }
    else if ( type == OFFFILE_TYPE )
    {
        return createOffFileMsg(result);
    }
    else if ( type == FILESRV_TRANSFER_TYPE )
    {
        return createFilesrvTransferMsg(result);
    }
    else
    {
        qDebug() << "Recive unknow type on MsgProcessor" << endl;
        return NULL;
    }
}

QQMsg *MsgProcessor::createFileMsg(const Json::Value &result) const
{
    Json::Value value = result["value"];

    QQFileMsg *file_msg = new QQFileMsg();
    file_msg->msg_id = value["msg_id"].asLargestInt();
    QString mode = QString::fromStdString(value["mode"].asString());
    if ( mode == "refuse" )
        file_msg->mode = QQFileMsg::kRefuse;
    else if ( mode == "recv" )
        file_msg->mode = QQFileMsg::kRecv;
    else if ( mode == "send_ack" )
        file_msg->mode = QQFileMsg::kSendAck;
    else
        qDebug() << "Recive unknow file message! Message mode: " << mode << endl;

    file_msg->from_id = QString::number(value["from_uin"].asLargestInt());
    file_msg->to_id = QString::number(value["to_uin"].asLargestInt());
    file_msg->msg_id2 = QString::number(value["msg_id2"].asLargestInt());
    file_msg->msg_type = value["msg_type"].asInt();
    file_msg->reply_ip = QString::number(value["reply_ip"].asLargestInt());
    file_msg->type = value["type"].asInt();
    file_msg->name = QString::fromStdString(value["name"].asString());
    file_msg->time = value["time"].asLargestInt();
    file_msg->session_id = value["session_id"].asLargestInt();
    file_msg->inet_ip = value["inet_ip"].asLargestInt();

    if ( !value["cancel_type"].empty() )
        file_msg->cancel_type = value["cancel_type"].asInt();

    return file_msg;
}

QQMsg *MsgProcessor::createOffFileMsg(const Json::Value &result) const
{
    Json::Value value = result["value"];

    QQOffFileMsg *offfile_msg = new QQOffFileMsg();
    offfile_msg->msg_id = value["msg_id"].asLargestInt();
    offfile_msg->rkey = QString::fromStdString(value["rkey"].asString());
    offfile_msg->ip = QString::fromStdString(value["ip"].asString());
    offfile_msg->port = value["port"].asInt();
    offfile_msg->from_id = QString::number(value["from_uin"].asLargestInt());
    offfile_msg->size = value["size"].asInt();
    offfile_msg->name = QString::fromStdString(value["name"].asString());
    offfile_msg->expire_time = value["expire_time"].asLargestInt();
    offfile_msg->time = value["time"].asLargestInt();

    return offfile_msg;
}

QQMsg *MsgProcessor::createFilesrvTransferMsg(const Json::Value &result) const
{
    Json::Value value = result["value"];

    QQFilesrvTransferMsg *filesrv_msg = new QQFilesrvTransferMsg();
    filesrv_msg->file_count = value["file_count"].asInt();
    filesrv_msg->name = QString::fromStdString(value["file_infos"][0]["file_name"].asString());
    filesrv_msg->file_status = value["file_infos"][0]["file_status"].asInt();
    filesrv_msg->pro_id = value["file_infos"][0]["pro_id"].asInt();

    filesrv_msg->from_id = QString::number(value["from_uin"].asLargestInt());
    filesrv_msg->to_id = QString::number(value["to_uin"].asLargestInt());
    filesrv_msg->now = value["now"].asLargestInt();
    filesrv_msg->operation = value["operation"].asInt();
    filesrv_msg->type = value["type"].asInt();
    filesrv_msg->lc_id = value["lc_id"].asInt();

    return filesrv_msg;
}

QQMsg *MsgProcessor::createFriendMsg(const Json::Value &result) const
{
    QQChatMsg *chat_msg = new QQChatMsg();
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
            if ( isChatContentEmpty(chat_msg, QString::fromStdString(content.asString())) )
                continue;

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
        chat_msg->msgs_.append(item);
    }

    return chat_msg;
}

QQMsg *MsgProcessor::createGroupMsg(const Json::Value &result) const
{
    QQGroupChatMsg *g_chat_msg = new QQGroupChatMsg();
    g_chat_msg->set_type(QQMsg::kGroup);
    g_chat_msg->send_uin_ = QString::number(result["value"]["send_uin"].asLargestInt());
    g_chat_msg->from_uin_ = QString::number(result["value"]["from_uin"].asLargestInt());
    g_chat_msg->to_uin_ = QString::number(result["value"]["to_uin"].asLargestInt());
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
            if ( isChatContentEmpty(g_chat_msg, QString::fromStdString(content.asString())) )
                continue;

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
                if ( content[0].asString() == "cface_idx" )
                    continue;

                item.set_type(QQChatItem::kGroupChatImg);
                item.set_content(QString::fromStdString(content[1]["name"].asString()));
                item.set_file_id(QString::number(content[1]["file_id"].asLargestInt()));
                QString server = QString::fromStdString(content[1]["server"].asString());
                int ip_end_idx = server.indexOf(":");
                item.set_server_ip(server.mid(0, ip_end_idx));
                item.set_server_port(server.mid(ip_end_idx+1));
            }
        }
        g_chat_msg->msgs_.append(item);
    }
    return g_chat_msg;
}

QQMsg *MsgProcessor::createSessMsg(const Json::Value &result) const
{
    QQSessChatMsg *chat_msg = new QQSessChatMsg();
    chat_msg->set_type(QQMsg::kSess);
    chat_msg->from_uin_ = QString::number(result["value"]["from_uin"].asLargestInt());
    chat_msg->gid_ = QString::number(result["value"]["id"].asLargestInt());
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
            if ( isChatContentEmpty(chat_msg, QString::fromStdString(content.asString())) )
                continue;

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
                //临时会话不支持图片发送
                continue;
            }
            /*
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
            */
        }
        chat_msg->msgs_.append(item);
    }

    return chat_msg;
}

QQMsg *MsgProcessor::createBuddiesStatusChangeMsg(const Json::Value &result) const
{
    QQStatusChangeMsg *status_msg = new QQStatusChangeMsg();
    status_msg->set_type(QQMsg::kBuddiesStatusChange);
    status_msg->uin_ = QString::number(result["value"]["uin"].asLargestInt());
    status_msg->client_type_ = (ContactClientType)result["value"]["client_type"].asInt();
    QString status_str = QString::fromStdString(result["value"]["status"].asString());
    status_msg->status_ = QQUtility::stringToStatus(status_str);

    return status_msg;
}

QQMsg *MsgProcessor::createSystemGroupMsg(const Json::Value &result) const
{
    QQSystemGMsg *system_g_msg = new QQSystemGMsg();
    system_g_msg->set_type(QQMsg::kSystemG);

    Json::Value msg_value = result["value"];
    system_g_msg->msg_id_ = QString::number(msg_value["msg_id"].asInt());
    system_g_msg->from_uin = QString::number(msg_value["from_uin"].asLargestInt());
    system_g_msg->to_uin = QString::number(msg_value["from_uin"].asLargestInt());
    system_g_msg->msg_id2_ = QString::number(msg_value["msg_id2"].asInt());
    system_g_msg->sys_g_type =QString::fromStdString(msg_value["type"].asString());
    system_g_msg->gcode_ = QString::number(msg_value["gcode"].asLargestInt());
    system_g_msg->t_gcode_ = QString::number(msg_value["t_gcode"].asLargestInt());
    system_g_msg->request_uin = QString::number(msg_value["request_uin"].asLargestInt());
    system_g_msg->t_request_uin_ = QString::fromStdString(msg_value["t_request_uin"].asString());
    system_g_msg->msg_ = QString::fromStdString(msg_value["msg"].asString());

    if ( !msg_value["op_type"].empty() )
        system_g_msg->op_type = msg_value["op_type"].asInt();
    if ( !msg_value["admin_uin"].empty() )
        system_g_msg->admin_uin = QString::number(msg_value["admin_uin"].asLargestInt());
    if ( !msg_value["new_member"].empty() )
        system_g_msg->new_member = QString::number(msg_value["new_member"].asLargestInt());
    if ( !msg_value["old_member"].empty() )
        system_g_msg->old_member = QString::number(msg_value["old_member"].asLargestInt());

    return system_g_msg;
}

QQMsg *MsgProcessor::createSystemMsg(const Json::Value &result) const
{
    QQSystemMsg *system_msg = new QQSystemMsg();
    system_msg->set_type(QQMsg::kSystem);
    system_msg->systemmsg_type_ = QString::fromStdString(result["value"]["type"].asString());
    system_msg->from_ = QString::number(result["value"]["from_uin"].asLargestInt());
    system_msg->account_ = QString::number(result["value"]["account"].asLargestInt());
    system_msg->msg_ = QString::fromStdString(result["value"]["msg"].asString());
    system_msg->status_ = (ContactStatus)(result["value"]["stat"].asInt());

    return system_msg;
}

bool MsgProcessor::isChatContentEmpty(const QQChatMsg *msg, const QString &content) const
{
    /*
     * 对方用webqq发送图片的时候,会在图片后自动上" "或者"\n "文本,
     * 这里判断这种情况,并将额外的" "或者"\n "去掉,不予显示.
     */
    if ( msg->msgs_.size() == 0 )
        return false;

    QQChatItem::ChatItemType type = msg->msgs_[msg->msgs_.size()-1].type();
    if ( type == QQChatItem::kFriendOffpic || type == QQChatItem::kGroupChatImg )
    {    
        if ( content.size() <= 2 )
        {
            return content[0] == ' ' || content[0] == '\n' || content[0] == '\r';
        }
    }
    return false;
}

void MsgProcessor::dispatchMsg(QVector<QQMsg *> &msgs) 
{
    foreach ( QQMsg *temp, msgs )
    {
        ShareQQMsgPtr msg(temp);
        if ( msg.isNull() )
            continue;

        switch ( msg->type() )
        {
            case QQMsg::kBuddiesStatusChange:
                emit contactStatusChanged(msg->sendUin(),  msg->status(), msg->client_type());
                break;
            case QQMsg::kFriend:
                emit newFriendChatMsg(msg);
                break;
            case QQMsg::kSess:
                emit newSessChatMsg(msg);
                break;
            case QQMsg::kGroup:
                emit newGroupChatMsg(msg);
                break;
            case QQMsg::kSystem:
                emit newSystemMsg(msg);
                break;
            case QQMsg::kSystemG:
                emit newSystemGMsg(msg);
                break;
            case QQMsg::kFile:
                emit newFileMsg(msg);
                break;
            case QQMsg::kOffFile:
                emit newOffFileMsg(msg);
                break;
            case QQMsg::kFilesrvTransfer:
                emit newFilesrvTransferMsg(msg);
                break;
            default:
                qDebug() << "recived unknown message!" << endl;
                break;
        }
    }
}

void MsgProcessor::stop()
{
    message_queue_.clear();
    quit();
}

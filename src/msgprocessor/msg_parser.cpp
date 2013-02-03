#include "msg_parser.h"

#include "json/json.h"

QQMsg *MsgParser::parse(const QByteArray &raw_msg, QVector<QQMsg *> &out_msgs)
{
    Json::Reader reader;
    Json::Value root;

    int idx = raw_msg.indexOf("\r\n\r\n");
    unparse_msg = raw_msg.mid(idx+4);

    if (!reader.parse(QString(unparse_msg).toStdString(), root, false))
        return NULL;

    QQMsg *msg = createMsg(type, result);
}


QQMsg* MsgProcessor::createMsg(QString type, const Json::Value result)
{
	if (type == "message")
	{
		return createFriendMsg(result);
	}
    else if (type ==  "group_message")
    {
        return createGroupMsg(result);
    }
    else if ( type == "sess_message" )
    {
        return createSessMsg(result);
    }
    else if (type == "buddies_status_change")
    {
        return createBuddiesStatusChangeMsg(result);
    }
    else if (type == "sys_g_msg")
    {
        return createSystemGroupMsg(result);
    }
    else if (type == "system_message")
    {
        return createSystemMsg(result);
    }
    else
    {
        qDebug()<<"unknow type"<<endl;
        return NULL;
    }
}

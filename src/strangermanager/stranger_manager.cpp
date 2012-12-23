#include "stranger_manager.h"

#include <QDebug>

#include "json/json.h"

#include "core/talkable.h"
#include "core/qqmsg.h"
#include "msgprocessor/msg_processor.h"
#include "qq_protocol/qq_protocol.h"
#include "qq_protocol/request_jobs/job_base.h"

StrangerManager *StrangerManager::instance_ = NULL;

StrangerManager::StrangerManager()
{
	connect(MsgProcessor::instance(), SIGNAL(newSystemMsg(ShareQQMsgPtr)), this, SLOT(onNewSystemMsg(ShareQQMsgPtr)));
	connect(MsgProcessor::instance(), SIGNAL(newSystemGMsg(ShareQQMsgPtr)), this, SLOT(onNewSystemMsg(ShareQQMsgPtr)));
}


StrangerManager::~StrangerManager()
{
	clean();
	instance_ = NULL;
}


bool StrangerManager::hasStrangerInfo(QString id) const
{
	if ( strangerInfo(id) )
		return true;

	return false;
}


void StrangerManager::parseStranger(const QByteArray &array)
{
	/*
	 * argument array example:
	 * "{"retcode":0,"result":{"face":0,"birthday":{"month":0,"year":0,"day":0},"phone":"","occupation":"","allow":1,"college":"","uin":2083567839,"blood":0,"constel":0,"homepage":"","stat":10,"country":"中国","city":"湛江","personal":"","nick":"O湛狼O","shengxiao":0,"email":"","token":"66ce8c44fa446e03480a8ea092414913dea5d35b218ac4eb","client_type":41,"province":"广东","gender":"male","mobile":"-"}}" 
	 *
	 */

	Json::Reader reader;
	Json::Value root;

	if (!reader.parse(QString(array).toStdString(), root, false))
	{
		return;
	}

	QString id = QString::number(root["result"]["uin"].asLargestInt());
	QString name = QString::fromStdString(root["result"]["nick"].asString());
	ContactStatus status = (ContactStatus)root["result"]["stat"].asInt();
	ContactClientType type = (ContactClientType)root["result"]["client_type"].asInt();

	Contact *stranger = new Contact(id, name);
	stranger->setStatus(status);	
	stranger->setClientType(type);	
	
	strangers_.append(stranger);
}

Contact *StrangerManager::strangerInfo(QString id) const
{
	foreach ( Contact *info, strangers_ )
	{
		if ( info->id() == id )
			return info;
	}
	return NULL;
}

void StrangerManager::addStrangerInfo(Contact *info)
{
	strangers_.append(info);
}

void StrangerManager::onNewSessMsg(ShareQQMsgPtr msg)
{
/* 	Contact *stranger = strangerInfo(msg->talkTo()); */
/*     if ( stranger ) */
/*         return; */
/*  */
/*     if ( !contact && !Protocol::QQProtocol::instance()->isRequesting(msg->sendUin(), JT_StrangerInfo2) ) */
/*     { */
/*         StrangerInfo2RequestCallback *callback = new StrangerInfo2RequestCallback(msg->sendUin()); */
/*         connect(callback, SIGNAL(sigRequestDone(QString, Contact *)), this, SLOT(contactInfoRequestDone(QString, Contact *))); */
/*  */
/*         Protocol::QQProtocol::instance()->requestStrangerInfo2(msg->sendUin(), gid, callback); */
/*  */
/*         IconRequestCallback *icon_callback = new IconRequestCallback(msg->sendUin()); */
/*         connect(icon_callback, SIGNAL(sigRequestDone(QString, QByteArray)), this, SLOT(contactIconRequestDone(QString, QByteArray))); */
/*  */
/*         Protocol::QQProtocol::instance()->requestIconFor(msg->sendUin(),  icon_callback); */
/*     } */
}

void StrangerManager::onNewSystemMsg(ShareQQMsgPtr msg)
{
	Contact *stranger = strangerInfo(msg->sendUin());
	if ( stranger )
		return;

	QString gid = 0;
	if ( msg->type() == QQMsg::kSystemG )
	{
		QQSystemGMsg *sysg_msg = (QQSystemGMsg *)msg.data();	
		gid = sysg_msg->from_uin;
		if ( sysg_msg->sys_g_type == "group_leave" )
		{
			qDebug() << sysg_msg->sendUin() << "had leave group" << endl;
			return;
		}
	}

	if ( !Protocol::QQProtocol::instance()->isRequesting(msg->sendUin(), JT_StrangerInfo2) )
	{
		StrangerInfo2RequestCallback *callback = new StrangerInfo2RequestCallback(msg->sendUin());
		connect(callback, SIGNAL(sigRequestDone(QString, Contact *)), this, SLOT(onInfoRequestDone(QString, Contact *)));

		Protocol::QQProtocol::instance()->requestStrangerInfo2(msg->sendUin(), gid, callback);

		IconRequestCallback *icon_callback = new IconRequestCallback(msg->sendUin());
		connect(icon_callback, SIGNAL(sigRequestDone(QString, QByteArray)), this, SLOT(onIconRequestDone(QString, QByteArray)));

		Protocol::QQProtocol::instance()->requestIconFor(msg->sendUin(),  icon_callback);
	}
}

void StrangerManager::onInfoRequestDone(QString id, Contact *stranger)
{
	strangers_.append(stranger);
	emit newStrangerInfo(id, stranger);
}

void StrangerManager::onIconRequestDone(QString id, QByteArray icon_data)
{
	Contact *stranger = strangerInfo(id);
	stranger->setIcon(icon_data);

	QPixmap pix;
	pix.loadFromData(icon_data);

	emit newStrangerIcon(id, pix);
}

void StrangerManager::clean()
{
	foreach ( Contact *stranger, strangers_ )	
	{
		delete stranger;
		stranger = NULL;
	}
	strangers_.clear();
}

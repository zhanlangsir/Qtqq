#include "stranger_manager.h"

#include <QDebug>

#include "json/json.h"

#include "core/talkable.h"
#include "core/qqmsg.h"
#include "roster/roster.h"
#include "msgprocessor/msg_processor.h"
#include "protocol/qq_protocol.h"
#include "protocol/request_jobs/job_base.h"
#include "event_handle/event_handle.h"

StrangerManager *StrangerManager::instance_ = NULL;

StrangerManager::StrangerManager()
{
	connect(MsgProcessor::instance(), SIGNAL(newSystemMsg(ShareQQMsgPtr)), this, SLOT(onNewSystemMsg(ShareQQMsgPtr)));
	connect(MsgProcessor::instance(), SIGNAL(newSystemGMsg(ShareQQMsgPtr)), this, SLOT(onNewSystemMsg(ShareQQMsgPtr)));
    EventHandle::instance()->registerObserver(Protocol::ET_OnStrangerInfoDone, this);
    EventHandle::instance()->registerObserver(Protocol::ET_OnStrangerAvatarUpdate, this);
}


StrangerManager::~StrangerManager()
{
    EventHandle::instance()->removeObserver(Protocol::ET_OnStrangerInfoDone, this);
    EventHandle::instance()->removeObserver(Protocol::ET_OnStrangerAvatarUpdate, this);

	clean();
	instance_ = NULL;
}


bool StrangerManager::hasStrangerInfo(QString id) const
{
	if ( stranger(id) )
		return true;

	return false;
}


void StrangerManager::updateStranger(const QByteArray &array)
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
		qDebug() << "StrangerManager parse stranger failed" << endl;
		qDebug() << "error json:" << array << endl;
		return;
	}

	QString id = QString::number(root["result"]["uin"].asLargestInt());
	QString name = QString::fromStdString(root["result"]["nick"].asString());
	ContactStatus status = (ContactStatus)root["result"]["stat"].asInt();
	ContactClientType type = (ContactClientType)root["result"]["client_type"].asInt();

	Contact *stranger = this->stranger(id);
    stranger->setName(name);
	stranger->setStatus(status);	
	stranger->setClientType(type);	
}

Contact *StrangerManager::stranger(const QString &id) const
{
	foreach ( Contact *info, strangers_ )
	{
		if ( info->id() == id )
			return info;
	}
	return NULL;
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
    if ( Roster::instance()->talkable(msg->sendUin()) )
        return;

	Contact *stranger = this->stranger(msg->sendUin());
	if ( stranger )
		return;

    stranger = new Contact(msg->sendUin(), "", Talkable::kStranger);
	QString gid = 0;
    bool group_request = false;
	if ( msg->type() == QQMsg::kSystemG )
	{
		QQSystemGMsg *sysg_msg = (QQSystemGMsg *)msg.data();	
		gid = sysg_msg->from_uin;
        group_request = true;
		if ( sysg_msg->sys_g_type == "group_leave" || sysg_msg->sys_g_type == "group_join" )
		{
			return;
		}
	}

    strangers_.append(stranger);
    Protocol::QQProtocol::instance()->requestStrangerInfo2(stranger, gid, group_request);
    Protocol::QQProtocol::instance()->requestIconFor(stranger);
}

void StrangerManager::onNotify(Protocol::Event *event)
{
    switch ( event->type() )
    {
        case Protocol::ET_OnStrangerAvatarUpdate:
            {
                Contact *stranger = (Contact *)event->eventFor();
                stranger->setAvatar(event->data());
                emit newStrangerIcon(stranger->id(), stranger->avatar());
            }
            break;
        case Protocol::ET_OnStrangerInfoDone:
                updateStranger(event->data());
                QString id = event->eventFor()->id();
                emit newStrangerInfo(event->eventFor()->id(), this->stranger(id));
            break;
    }
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

Contact *StrangerManager::takeStranger(QString id)
{
    Contact *stranger = this->stranger(id); 
    if ( stranger )
    {
        strangers_.removeOne(stranger);
    }
    return stranger;
}

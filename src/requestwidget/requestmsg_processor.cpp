#include "requestmsg_processor.h"

#include <assert.h>

#include <QIcon>
#include <QAction>
#include <QDebug>

#include "core/qqmsg.h"
#include "roster/roster.h"
#include "strangermanager/stranger_manager.h"
#include "protocol/qq_protocol.h"
#include "trayicon/systemtray.h"
#include "requestwidget/friendrequestdlg.h"
#include "requestwidget/grouprequestdlg.h"
#include "msgprocessor/msg_processor.h"
#include "skinengine/qqskinengine.h"
#include "soundplayer/soundplayer.h"

RequestMsgProcessor *RequestMsgProcessor::instance_ = NULL;

static QString kFriendActionText = QObject::tr("Friend request: %1");
static QString kGroupActionText = QObject::tr("%1 request to enter group [%2]");

RequestMsgProcessor::RequestMsgProcessor()
{
	connect(MsgProcessor::instance(), SIGNAL(newSystemMsg(ShareQQMsgPtr)), this, SLOT(onNewSystemMsg(ShareQQMsgPtr)));
	connect(MsgProcessor::instance(), SIGNAL(newSystemGMsg(ShareQQMsgPtr)), this, SLOT(onNewSystemMsg(ShareQQMsgPtr)));
}

RequestMsgProcessor::~RequestMsgProcessor()
{
	stop();
}

void RequestMsgProcessor::onNewSystemMsg(ShareQQMsgPtr msg)
{
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
	Contact *contact = Roster::instance()->contact(msg->sendUin());
	if ( !contact )
	{
		StrangerManager *mgr = StrangerManager::instance();
		contact = mgr->stranger(msg->sendUin());
	}

	if ( !contact || contact->avatar().isNull() )
	{
		connect(StrangerManager::instance(), SIGNAL(newStrangerInfo(QString, Contact *)), this, SLOT(onNewStrangerInfo(QString, Contact *)));
		connect(StrangerManager::instance(), SIGNAL(newStrangerIcon(QString, QPixmap)), this, SLOT(onNewStrangerIcon(QString, QPixmap)));
	}

	createTrayNotify(msg, contact);
}

void RequestMsgProcessor::onActionTriggered()
{
	QAction *act = qobject_cast<QAction *>(sender());
	if ( !act )
		return;

	ShareQQMsgPtr msg = act->data().value<ShareQQMsgPtr>();
	switch ( msg->type() )
	{
		case QQMsg::kSystem:
			{
				Contact *contact = StrangerManager::instance()->stranger(msg->sendUin());
				StrangerManager::instance()->disconnect(this);

                const QQSystemMsg *sys_msg = static_cast<const QQSystemMsg*>(msg.data());
				FriendRequestDlg *dlg = new FriendRequestDlg(sys_msg->sendUin(), sys_msg->account_, sys_msg->msg_, contact);
				dlg->show();
			}
			break;

		case QQMsg::kSystemG:
			{
				Contact *contact = Roster::instance()->contact(msg->sendUin());
				if ( !contact )
					contact = StrangerManager::instance()->stranger(msg->sendUin());
				Group *group = Roster::instance()->group(msg->talkTo());
				assert(group);

				StrangerManager::instance()->disconnect(this);

				GroupRequestDlg *dlg = new GroupRequestDlg(msg, contact, group);
				dlg->show();
			}
			break;
		default:
			qDebug() << "Recive wrong Action triggerd on RequestMsgProcess!" << endl;
			return;
	}

	actions_.removeOne(act);
	SystemTrayIcon::instance()->removeAction(act);
	delete act;
	act = NULL;
}

void RequestMsgProcessor::createTrayNotify(ShareQQMsgPtr msg, Contact *stranger)
{
	QAction *action = NULL;
	if ( msg->type() == QQMsg::kSystem )
	{
		action = friendActionById(msg->sendUin());
	}
	else if ( msg->type() == QQMsg::kSystemG )
	{
		action = groupActionById(msg->sendUin());
	}
	else
	{
		qDebug() << "Unexpection msg type at RequestMsgProcessor::createTrayNotify!" << endl;
		return;
	}

	if ( !action )
	{
		QString name;
		QIcon icon;
		
		if ( msg->type() == QQMsg::kSystem )
		{
            QQSystemMsg *sys_msg = (QQSystemMsg*)msg.data();
			if ( stranger )
			{
				name = kFriendActionText.arg(stranger->name().isEmpty() ? sys_msg->account_ : stranger->name());
				QPixmap pix = stranger->avatar();
				if ( !pix.isNull() )
					icon.addPixmap(pix);
			}
			else
			{
				name = kFriendActionText.arg(sys_msg->account_);
			}
			if ( icon.isNull() )
			{
				icon.addPixmap(QPixmap(QQSkinEngine::instance()->skinRes("default_friend_avatar")));
			}
		}
		else if ( msg->type() == QQMsg::kSystemG )
		{
			if ( stranger )
			{
                Group *group = Roster::instance()->group(msg->talkTo());
                assert(group);
				name = kGroupActionText.arg(stranger->name()).arg(group->name());

				QPixmap pix = stranger->avatar();
				if ( !pix.isNull() )
					icon.addPixmap(pix);
			}
			else
			{
				QQSystemGMsg *sysg_msg = (QQSystemGMsg*)msg.data();
				name = kGroupActionText.arg(sysg_msg->t_request_uin_);
			}
			if ( icon.isNull() )
			{
				icon.addPixmap(QPixmap(QQSkinEngine::instance()->skinRes("default_group_avatar")));
			}
		}


		QAction *action = new QAction(icon, name, NULL);
		action->setData(QVariant::fromValue<ShareQQMsgPtr>(msg));
		actions_.append(action);
		SystemTrayIcon::instance()->addNotifyAction(action);
		connect(action, SIGNAL(triggered()), this, SLOT(onActionTriggered()));

        SoundPlayer::singleton()->play(SoundPlayer::kSystem);
	}
}

QAction *RequestMsgProcessor::friendActionById(const QString &id) const
{
	foreach ( QAction *act, actions_ )
	{
		ShareQQMsgPtr msg = act->data().value<ShareQQMsgPtr>();
		if ( msg->sendUin() == id && msg->type() == QQMsg::kSystem )
		{
			return act;
		}
	}
	return NULL;
}

QAction *RequestMsgProcessor::groupActionById(const QString &id) const
{
	foreach ( QAction *act, actions_ )
	{
		ShareQQMsgPtr msg = act->data().value<ShareQQMsgPtr>();
		if ( msg->sendUin() == id && msg->type() == QQMsg::kSystemG )
		{
			return act;
		}
	}
	return NULL;
}

void RequestMsgProcessor::onNewStrangerInfo(QString id, Contact *stranger)
{
	QAction *friendreq_act = friendActionById(id);		
	if ( friendreq_act )
	{
		friendreq_act->setText(kFriendActionText.arg(stranger->name()));
	}

	QAction *groupreq_act = groupActionById(id);
	if ( groupreq_act )
	{
		groupreq_act->setText(kGroupActionText.arg(stranger->name()));
	}
}

void RequestMsgProcessor::onNewStrangerIcon(QString id, QPixmap pix)
{

	QIcon icon;
	icon.addPixmap(pix);

	QAction *friendreq_act = friendActionById(id);		
	if ( friendreq_act )
	{
		friendreq_act->setIcon(icon);
	}

	QAction *groupreq_act = groupActionById(id);
	if ( groupreq_act )
	{
		groupreq_act->setIcon(icon);
	}
}

void RequestMsgProcessor::stop()
{
	SystemTrayIcon *trayicon = SystemTrayIcon::instance();
	foreach ( QAction *act, actions_ )
	{
		actions_.removeOne(act);
		trayicon->removeAction(act);

		delete act;
		act = NULL;
	}
}

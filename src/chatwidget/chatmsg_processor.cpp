#include "chatmsg_processor.h"

#include <assert.h>

#include <QAction>
#include <QPixmap>
#include <QIcon>
#include <QVariant>
#include <QDebug>

#include "chatwidget/qqchatdlg.h"
#include "roster/roster.h"
#include "core/talkable.h"
#include "core/qqmsglistener.h"
#include "chatwidget/chatdlg_manager.h"
#include "msgprocessor/msg_processor.h"
#include "skinengine/qqskinengine.h"
#include "soundplayer/soundplayer.h"
#include "trayicon/systemtray.h"

ChatMsgProcessor *ChatMsgProcessor::instance_ = NULL;

ChatMsgProcessor::ChatMsgProcessor()
{
    qRegisterMetaType<ContactStatus>("ContactStatus");

	connect(MsgProcessor::instance(), SIGNAL(newFriendChatMsg(ShareQQMsgPtr)), this, SLOT(onNewChatMsg(ShareQQMsgPtr)));
	connect(MsgProcessor::instance(), SIGNAL(newGroupChatMsg(ShareQQMsgPtr)), this, SLOT(onNewChatMsg(ShareQQMsgPtr)));
	connect(MsgProcessor::instance(), SIGNAL(newOffFileMsg(ShareQQMsgPtr)), this, SLOT(onNewOffFileMsg(ShareQQMsgPtr)));
	connect(ChatDlgManager::instance(), SIGNAL(activatedChatDlgChanged(QQChatDlg *, QQChatDlg *)), this, SLOT(onActivatedChatDlgChanged(QQChatDlg *, QQChatDlg *)));
}

ChatMsgProcessor::~ChatMsgProcessor()
{
	stop();
}

void ChatMsgProcessor::onNewChatMsg(ShareQQMsgPtr msg)
{
	QQMsgListener *listener = listenerById(msg->talkTo());

	if ( listener )
	{
		listener->showMsg(msg);
	}
	else
	{
		old_msgs_.push_back(msg);
		createTrayNotify(msg);

		ChatDlgManager *chatdlg_mgr = ChatDlgManager::instance();
		if ( chatdlg_mgr->isOpening(msg->talkTo()) )
		{
			chatdlg_mgr->notifyDlgById(msg->talkTo());
		}

        SoundPlayer::singleton()->play(SoundPlayer::kMsg);
	}
}

void ChatMsgProcessor::onNewOffFileMsg(ShareQQMsgPtr msg)
{
    onNewChatMsg(msg);
}

void ChatMsgProcessor::registerListener(QQMsgListener *listener)
{
	if ( listenerById(listener->id()) )
		return;

	QVector<ShareQQMsgPtr> old_msgs;
	getOldMsgs(listener->id(), old_msgs);
	listener->showOldMsg(old_msgs);
	listener_.append(listener);
}

void ChatMsgProcessor::removeListener(QQMsgListener *listener)
{
	listener_.removeOne(listener);
}

void ChatMsgProcessor::getOldMsgs(const QString &id, QVector<ShareQQMsgPtr> &msgs)
{
	QList<ShareQQMsgPtr> msgs_copy = old_msgs_;

	foreach( ShareQQMsgPtr msg, msgs_copy )
	{
		if (msg->talkTo() == id)
		{
			msgs.append(msg);
			old_msgs_.removeOne(msg);
		}
	}
}

QQMsgListener *ChatMsgProcessor::listenerById(const QString &id) const
{
	foreach ( QQMsgListener *listener, listener_ )
	{
		if ( listener->id() == id )
			return listener;
	}
	return NULL;
}

QAction *ChatMsgProcessor::actionById(const QString &id) const
{
	foreach ( QAction *act, actions_ )
	{
		QString act_id = act->data().toString();
		if ( act_id == id )
			return act;
	}
	return NULL;
}

void ChatMsgProcessor::createTrayNotify(ShareQQMsgPtr msg)
{
	QAction *action = actionById(msg->talkTo());
	if ( action )
	{
		QString text = action->text();
		int idx = text.indexOf('\t', 0);
		// 仅一条未读
		if (idx == -1)
		{
			action->setText(text + "\t(2)");
		}
		else
		{
			QString number;
			QString newText = text;
			newText.truncate(idx);

			for(int i=idx+2; i<text.length(); ++i)
			{
				if (text.at(i) == ')')
					break;
				number.append(text.at(i));
			}

			uint count = number.toUInt();
			action->setText(newText + "\t(" + QString::number(count + 1) + ")");
		}
	}
	else
	{
		Roster *roster = Roster::instance();
		Talkable *talkable = roster->talkable(msg->talkTo());

		/*
		 * I don't know why i can receive some group message which send from 
		 * the group that i had not join in. So, just skip it. 
		 */
		if ( !talkable )
			return;

		QPixmap pix = talkable->avatar();
		QIcon icon;
		if ( !pix.isNull() )
			icon.addPixmap(pix);
		else
		{
			if ( talkable->type() == Talkable::kContact )
			{
				icon.addPixmap(QPixmap(QQSkinEngine::instance()->skinRes("default_friend_avatar")));
			}
			else if ( talkable->type() == Talkable::kGroup )
			{
				icon.addPixmap(QPixmap(QQSkinEngine::instance()->skinRes("default_group_avatar")));
			}
		}

		action = new QAction(icon, talkable->name(), NULL);

		action->setData(msg->talkTo());

		actions_.append(action);
		SystemTrayIcon::instance()->addNotifyAction(action);	
		connect(action, SIGNAL(triggered()), this, SLOT(onActionTriggered()));
	}
}

void ChatMsgProcessor::onActionTriggered()
{
	QAction *act = qobject_cast<QAction *>(sender());
	if ( !act )
		return;

	QString id = act->data().toString();
	removeAction(act);

	ShareQQMsgPtr msg;
	foreach ( ShareQQMsgPtr temp, old_msgs_ )	
	{
		if ( temp->talkTo() == id )
		{
			msg = temp;
			break;
		}
	}

	assert(!msg.isNull());

	switch ( msg->type() )
	{
		case QQMsg::kFriend:
			ChatDlgManager::instance()->openFriendChatDlg(msg->talkTo());
			break;
		case QQMsg::kGroup:
			ChatDlgManager::instance()->openGroupChatDlg(msg->talkTo(), msg->gCode());
			break;
		case QQMsg::kSess:
			ChatDlgManager::instance()->openSessChatDlg(msg->talkTo(), msg->gid());
			break;
        case QQMsg::kOffFile:
			ChatDlgManager::instance()->openFriendChatDlg(msg->talkTo());
			break;
		default:
			qDebug() << "Recive wrong action triggered on ChatMsgProcess!" << endl;
			return;
	}
}

void ChatMsgProcessor::onActivatedChatDlgChanged(QQChatDlg *before, QQChatDlg *after)
{
	if ( before )
	{
		removeListener(before);
	}

	if ( after )
	{
		registerListener(after);

		QAction *act = actionById(after->id());
		if ( act )
			removeAction(act);
	}
}


void ChatMsgProcessor::removeAction(QAction *act)
{
	actions_.removeOne(act);
	SystemTrayIcon::instance()->removeAction(act);

	delete act;
	act = NULL;
}


void ChatMsgProcessor::stop()
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

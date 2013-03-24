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
#include "strangermanager/stranger_manager.h"
#include "trayicon/systemtray.h"

ChatMsgProcessor *ChatMsgProcessor::instance_ = NULL;

ChatMsgProcessor::ChatMsgProcessor()
{
    qRegisterMetaType<ContactStatus>("ContactStatus");

	connect(MsgProcessor::instance(), SIGNAL(newFriendChatMsg(ShareQQMsgPtr)), this, SLOT(onNewChatMsg(ShareQQMsgPtr)));
	connect(MsgProcessor::instance(), SIGNAL(newGroupChatMsg(ShareQQMsgPtr)), this, SLOT(onNewChatMsg(ShareQQMsgPtr)));
	connect(MsgProcessor::instance(), SIGNAL(newSessChatMsg(ShareQQMsgPtr)), this, SLOT(onNewChatMsg(ShareQQMsgPtr)));
	connect(MsgProcessor::instance(), SIGNAL(newOffFileMsg(ShareQQMsgPtr)), this, SLOT(onNewOffFileMsg(ShareQQMsgPtr)));
	connect(ChatDlgManager::instance(), SIGNAL(activatedChatDlgChanged(QQChatDlg *, QQChatDlg *)), this, SLOT(onActivatedChatDlgChanged(QQChatDlg *, QQChatDlg *)));
    connect(StrangerManager::instance(), SIGNAL(newStrangerInfo(QString, Contact *)), this, SLOT(onNewStrangerInfo(QString, Contact *)));
    connect(StrangerManager::instance(), SIGNAL(newStrangerIcon(QString, QPixmap)), this, SLOT(onNewStrangerIcon(QString, QPixmap)));
}

ChatMsgProcessor::~ChatMsgProcessor()
{
	stop();
}

void ChatMsgProcessor::onNewStrangerInfo(QString id, Contact *stranger)
{
    QAction *action = actionById(id);
    if ( !action )
        return;

    action->setText(stranger->name());
}

void ChatMsgProcessor::onNewStrangerIcon(QString id, QPixmap pix)
{
	QIcon icon;
	icon.addPixmap(pix);

	QAction *action = actionById(id);		
	if ( action )
	{
		action->setIcon(icon);
	}
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
		if ( msg->talkTo() == id )
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
        Talkable *talkable = NULL;
        if ( msg->type() == QQMsg::kSess )
        {
            Group *group = Roster::instance()->group(msg->gid());
            talkable = group->member(msg->sendUin());
        }
        else
            talkable = roster->talkable(msg->talkTo());

        if ( !talkable && msg->type() == QQMsg::kGroup )
            return;
        else if ( !talkable )
        {
            if ( msg->type() == QQMsg::kFriend )
                talkable = StrangerManager::instance()->addStranger(msg->sendUin(), QString(), Talkable::kStranger);
            else
                talkable = StrangerManager::instance()->addStranger(msg->sendUin(), msg->gid(), Talkable::kSessStranger);
        }

		QPixmap pix = talkable ? talkable->avatar() : QPixmap(QQSkinEngine::instance()->skinRes("default_friend_avatar"));
		QIcon icon;
		if ( !pix.isNull() )
			icon.addPixmap(pix);
		else
		{
			if ( talkable->type() == Talkable::kContact || talkable->type() == Talkable::kStranger || talkable->type() == Talkable::kSessStranger )
			{
				icon.addPixmap(QPixmap(QQSkinEngine::instance()->skinRes("default_friend_avatar")));
			}
			else if ( talkable->type() == Talkable::kGroup )
			{
				icon.addPixmap(QPixmap(QQSkinEngine::instance()->skinRes("default_group_avatar")));
			}
		}

        QString name = talkable ? talkable->markname() : msg->talkTo();
		action = new QAction(icon, name, NULL);

        if ( msg->type() == QQMsg::kSess )
            action->setData(msg->sendUin());
        else
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
			ChatDlgManager::instance()->openSessChatDlg(msg->sendUin(), msg->gid());
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

	act->deleteLater();
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

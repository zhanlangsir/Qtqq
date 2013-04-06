#include "requestmsg_processor.h"

#include <assert.h>

#include <QIcon>
#include <QAction>
#include <QLabel>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
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
	connect(MsgProcessor::instance(), SIGNAL(newSystemGMsg(ShareQQMsgPtr)), this, SLOT(onNewSystemGMsg(ShareQQMsgPtr)));
}

RequestMsgProcessor::~RequestMsgProcessor()
{
	stop();
}

void RequestMsgProcessor::onNewSystemGMsg(ShareQQMsgPtr msg)
{
    QQSystemGMsg *sysg_msg = (QQSystemGMsg *)msg.data();	
    Group *group = Roster::instance()->group(msg->talkTo());

    /*
     * 因为每次的gid,uin等都是在变化的,所以如果有其他人申请入群,然后你的这个时候下线,
     * 再上线,你的gid已经更新了,但是收到的申请消息里的gid还是上一次的,所以找不到对应
     * 的群,这里直接输出错误消息,然后不做处理
     */
    if ( !group )
    {
        qDebug() << "Recive wrong group requester" << endl;
        return;
    }

    if ( sysg_msg->sys_g_type == "group_request_join" )
    {
        createTrayNotify(msg, getContactFromSystemMsg(msg->sendUin()));
    }
    else
    {

        QPixmap group_avatar = group->avatar().isNull() ?  QPixmap(QQSkinEngine::instance()->skinRes("default_group_avatar")) : group->avatar();
        QString msg = "";
        switch ( sysg_msg->op_type )
        {
            //你的管理员身份被删除
            case 0:
                return;  //这个提示有错误
                if ( sysg_msg->sys_g_type == "group_admin_op" )
                {
                    msg = tr("You administrator privileges in the group %1 is deleted").arg(group->markname());
                }
                break;
                //你被设置为管理员
            case 1:
                return;  //这个提示有错误
                {
                    msg = tr("You are set to the administrator of the group %1.").arg(group->markname());
                }
                break;
                //其他管理员允许入群
            case 2:
                {
                    if ( sysg_msg->sys_g_type == "group_leave" )
                    {
                        Contact *leave_member = group->member(sysg_msg->old_member);
                        QString leave_name = leave_member ? leave_member->markname() : sysg_msg->old_member;
                        group->removeMember(sysg_msg->old_member);
                        msg = tr("Member %1 has leave group %2").arg(leave_name).arg(group->markname());
                    }
                    else if ( sysg_msg->sys_g_type == "group_join" )
                    {
                        Contact *admin = group->member(sysg_msg->admin_uin);
                        QString admin_name = admin ? admin->markname() : sysg_msg->admin_uin;

                        Contact *new_member = Roster::instance()->contact(sysg_msg->new_member);
                        if ( !new_member )
                        {
                            new_member = StrangerManager::instance()->stranger(sysg_msg->new_member);
                        }

                        if( new_member )
                        {
                            msg = tr("Administrator %1 allow %2 to join group %3").arg(admin_name).arg(new_member->markname()).arg(group->markname());

                            Contact *clone = new_member->clone();
                            clone->setCategory(NULL);
                            group->addMember(clone);
                        }
                        else
                        {
                            msg = tr("Administrator %1 allow %2 to join group %3").arg(admin_name).arg(sysg_msg->new_member).arg(group->markname());
                            group->addMember(sysg_msg->new_member);
                        }
                    }
                }
                break;
                //其他管理员删除群成员
            case 3:
                {
                    Contact *admin = group->member(sysg_msg->admin_uin);
                    QString admin_name = admin ? admin->markname() : sysg_msg->admin_uin;
                    msg = tr("Administrator %1 deleted member %2 from group %3").arg(admin_name).arg(sysg_msg->old_member).arg(group->markname());
                }
                break;
            default:
                {
                    qDebug() << "Recive unknow group msg" << endl;
                    return;
                }
        }

        showMessageBox(group_avatar, msg);
    }
}

void RequestMsgProcessor::showMessageBox(const QPixmap &pix, const QString &msg)
{
    QDialog *dlg = new QDialog();
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    QLabel *avatar_label = new QLabel("");
    avatar_label->setPixmap(pix);

    QLabel *msg_label = new QLabel(msg);

    QHBoxLayout *h_layout = new QHBoxLayout();
    h_layout->addWidget(avatar_label);
    h_layout->addWidget(msg_label);

    QPushButton *ok_btn = new QPushButton(tr("Ok"));
    connect(ok_btn, SIGNAL(clicked()), dlg, SLOT(close()));

    QVBoxLayout *main_layout = new QVBoxLayout(dlg);
    main_layout->addLayout(h_layout); 
    main_layout->addWidget(ok_btn); 
    dlg->setLayout(main_layout);

    dlg->move((QApplication::desktop()->width() - dlg->width()) /2, (QApplication::desktop()->height() - dlg->height()) /2);
    dlg->show();
}

void RequestMsgProcessor::onNewSystemMsg(ShareQQMsgPtr msg)
{
	createTrayNotify(msg, getContactFromSystemMsg(msg->sendUin()));
}

Contact *RequestMsgProcessor::getContactFromSystemMsg(const QString &id)
{
	Contact *contact = Roster::instance()->contact(id);
	if ( !contact )
	{
		StrangerManager *mgr = StrangerManager::instance();
		contact = mgr->stranger(id);
	}

	if ( !contact || contact->avatar().isNull() )
	{
		connect(StrangerManager::instance(), SIGNAL(newStrangerInfo(QString, Contact *)), this, SLOT(onNewStrangerInfo(QString, Contact *)));
		connect(StrangerManager::instance(), SIGNAL(newStrangerIcon(QString, QPixmap)), this, SLOT(onNewStrangerIcon(QString, QPixmap)));
	}

    return contact;
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
                Contact *contact = Roster::instance()->contact(msg->sendUin());
                if ( !contact )
                    contact = StrangerManager::instance()->stranger(msg->sendUin());
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
                requesting_groups_.remove(msg->sendUin());

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
            Group *group = Roster::instance()->group(msg->talkTo());
            /*
             * 因为每次的gid,uin等都是在变化的,所以如果有其他人申请入群,然后你的这个时候下线,
             * 再上线,你的gid已经更新了,但是收到的申请消息里的gid还是上一次的,所以找不到对应
             * 的群,这里直接输出错误消息,然后不做处理
             */
            if ( !group )
            {
                qDebug() << "Recive wrong group requester" << endl;
                return;
            }

			if ( stranger )
			{
				name = kGroupActionText.arg(stranger->name()).arg(group->markname());
                if ( stranger->name().isEmpty() )
                {
                    requesting_groups_[msg->sendUin()] = group->markname();
                }

				QPixmap pix = stranger->avatar();
				if ( !pix.isNull() )
					icon.addPixmap(pix);
			}
			else
			{
				QQSystemGMsg *sysg_msg = (QQSystemGMsg*)msg.data();
				name = kGroupActionText.arg(sysg_msg->t_request_uin_).arg(group->markname());
                requesting_groups_[msg->sendUin()] = group->markname();
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
		groupreq_act->setText(kGroupActionText.arg(stranger->name()).arg(requesting_groups_[id]));
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

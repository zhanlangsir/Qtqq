#include "chatmanager.h"

#include <QDesktopWidget>

#include "../qqchatdlg.h"
#include "../mainwindow.h"
#include "../groupchatdlg.h"
#include "../friendchatdlg.h"
#include "../sesschatdlg.h"
#include "../frienditemmodel.h"
#include "../groupitemmodel.h"
#include "groupmsgencoder.h"
#include "friendmsgencoder.h"
#include "sessmsgencoder.h"
#include "nameconvertor.h"
#include "msgcenter.h"
#include "qqitem.h"
#include "../qqiteminfohelper.h"
#include "json/json.h"

ChatManager::ChatManager(MainWindow *main_win, const NameConvertor *convertor) :
    main_win_(main_win),
    convertor_(convertor)
{
}

ChatManager::~ChatManager()
{
    foreach(QQChatDlg *dlg, opening_chatdlg_)
    {
        dlg->close();
    }
}

void ChatManager::openFriendChatDlg(const QString &id)
{
	if ( isOpening(id) || id.isEmpty() )
        return;

    QQChatDlg *dlg = NULL;
    QString avatar_path = getFriendAvatarPath(id);

    dlg= new FriendChatDlg(id, convertor_->convert(id), avatar_path);
    MsgEncoder *encoder = new FriendMsgEncoder(dlg);
    dlg->setMsgEncoder(encoder);

    connect(dlg, SIGNAL(chatFinish(QQChatDlg*)), this, SLOT(closeChatDlg(QQChatDlg*)));
    connect(dlg, SIGNAL(msgSended(QString,bool)), main_win_->recentModel(), SLOT(improveItem(QString)));

    main_win_->msgCenter()->registerListener(dlg);
    opening_chatdlg_.append(dlg);
    dlg->move((QApplication::desktop()->width() - dlg->width()) /2, (QApplication::desktop()->height() - dlg->height()) /2);
    dlg->show();

    main_win_->msgTip()->removeItem(id);
}

void ChatManager::openGroupChatDlg(QString id, QString gcode)
{
   if ( isOpening(id) )
      return; 

    QQChatDlg *dlg = NULL;

    QString avatar_path = getGroupAvatarPath(id);

    dlg = new GroupChatDlg(id, convertor_->convert(id), gcode, avatar_path, this, main_win_);
    MsgEncoder *encoder = new GroupMsgEncoder(dlg);
    dlg->setMsgEncoder(encoder);

    connect(dlg, SIGNAL(chatFinish(QQChatDlg*)), this, SLOT(closeChatDlg(QQChatDlg*)));
    connect(dlg, SIGNAL(msgSended(QString,bool)), main_win_->recentModel(), SLOT(improveItem(QString)));

    main_win_->msgCenter()->registerListener(dlg);

    opening_chatdlg_.append(dlg);
    dlg->move((QApplication::desktop()->width() - dlg->width()) /2, (QApplication::desktop()->height() - dlg->height()) /2);
    dlg->show();

    main_win_->msgTip()->removeItem(id);
}

void ChatManager::openSessChatDlg(QString id, QString group_id) 
{
    if ( isOpening(id) )
        return; 

    QQChatDlg *dlg = NULL;

	QByteArray info = QQItemInfoHelper::getStrangetInfo2(id, group_id);
	info = info.mid(info.indexOf("\r\n\r\n") + 4);

	Json::Reader reader;
	Json::Value root;

	if (!reader.parse(QString(info).toStdString(), root, false))
	{
		return;
	}

	QString name = QString::fromStdString(root["result"]["nick"].asString());
	//暂时不明白token的作用
	//QString	token = QString::fromStdString(root["result"]["token"].asString());

	QQItem *ginfo = main_win_->groupModel()->find(group_id);
    QString avatar_path = QQAvatarRequester::requestOne(1, id, QQGlobal::tempPath());

	dlg = new SessChatDlg(id, name, avatar_path, ginfo->name());
	MsgEncoder *encoder = new SessMsgEncoder(dlg, ginfo->gCode(), GroupChatDlg::getMsgSig(group_id, id));
    dlg->setMsgEncoder(encoder);

    connect(dlg, SIGNAL(chatFinish(QQChatDlg*)), this, SLOT(closeChatDlg(QQChatDlg*)));
    connect(dlg, SIGNAL(msgSended(QString,bool)), main_win_->recentModel(), SLOT(improveItem(QString)));

    main_win_->msgCenter()->registerListener(dlg);

    opening_chatdlg_.append(dlg);
    dlg->move((QApplication::desktop()->width() - dlg->width()) /2, (QApplication::desktop()->height() - dlg->height()) /2);
    dlg->show();

    main_win_->msgTip()->removeItem(id);
}

bool ChatManager::isOpening(const QString &id) const
{
    foreach(QQChatDlg *chatdlg, opening_chatdlg_)
    {
        if (chatdlg->id() == id)
            return true;
    }
    return false;
}

QString ChatManager::getFriendAvatarPath(const QString &id) const
{
    QQItem *item = main_win_->friendModel()->find(id);

    if (item)
        return  item->avatarPath();

    return "";
}

QString ChatManager::getGroupAvatarPath(const QString &id) const
{
    QQItem *item = main_win_->groupModel()->find(id);

    if (item)
        return  item->avatarPath();

    return "";
}

QQChatDlg* ChatManager::findChatDlg(QString id) const
{
    foreach ( QQChatDlg *dlg, opening_chatdlg_ )
    {
        if ( dlg->id() == id )
            return dlg;
    }
    return NULL;
}

void ChatManager::closeChatDlg(QQChatDlg *listener)
{
    opening_chatdlg_.remove(opening_chatdlg_.indexOf(listener));
    main_win_->msgCenter()->removeListener(listener);
    listener->deleteLater();
}

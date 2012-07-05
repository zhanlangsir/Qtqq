#include "chatmanager.h"

#include <QDesktopWidget>

#include "../qqchatdlg.h"
#include "../mainwindow.h"
#include "../groupchatdlg.h"
#include "../friendchatdlg.h"
#include "../groupchatdlg.h"
#include "../frienditemmodel.h"
#include "../groupitemmodel.h"
#include "groupmsgencoder.h"
#include "friendmsgencoder.h"
#include "sessmsgencoder.h"
#include "nameconvertor.h"
#include "msgcenter.h"
#include "qqitem.h"


const QString ChatManager::kFriendSendUrl = "/channel/send_buddy_msg2";
const QString ChatManager::kGroupSendUrl =  "/channel/send_qun_msg2";
const QString ChatManager::kSessSendUrl = "/channel/send_sess_msg2";

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

void ChatManager::openFriendChatDlg(QString id)
{
    if ( isOpening(id) )
        return;

    QQChatDlg *dlg = NULL;
    QString avatar_path = getFriendAvatarPath(id);

    dlg= new FriendChatDlg(id, convertor_->convert(id), avatar_path, kFriendSendUrl);
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

    dlg = new GroupChatDlg(id, convertor_->convert(id), gcode, avatar_path, kGroupSendUrl, this, main_win_);
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

    GroupChatDlg* group_dlg = static_cast<GroupChatDlg *>(findChatDlg(group_id));
    QQItem *info = group_dlg->model()->find(id);

    if ( !info )
        return;

    dlg = new FriendChatDlg(id, info->markName(), info->avatarPath(), kSessSendUrl);
    MsgEncoder *encoder = new SessMsgEncoder(dlg, group_dlg->code(), group_dlg->msgSig());
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

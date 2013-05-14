#include "chatdlg_manager.h"

#include <cassert>

#include <QDesktopWidget>
#include <QApplication>
#include <QAction>

#include "json/json.h"

#include "chatwidget/friendchatdlg.h"
#include "chatwidget/groupchatdlg.h"
#include "chatwidget/qqchatdlg.h"
#include "chatwidget/sesschatdlg.h"
#include "chatwidget/chatmsg_processor.h"
#include "rostermodel/recent_model.h"
#include "roster/roster.h"
#include "chatwidget/tabwindow.h"
#include "strangermanager/stranger_manager.h"
#include "rostermodel/roster_index.h"
#include "rosterview/rosterview.h"
#include "utils/menu.h"
#include "mainwindow.h"
#include "qtqq.h"

ChatDlgManager* ChatDlgManager::instance_ = NULL;

ChatDlgManager::ChatDlgManager()
{
    tab_win_ = new TabWindow();
    connect(tab_win_, SIGNAL(activatedPageChanged(QQChatDlg *, QQChatDlg *)), this, SIGNAL(activatedChatDlgChanged(QQChatDlg *, QQChatDlg *)));
    tab_win_->move((QApplication::desktop()->width() - tab_win_->width()) /2, (QApplication::desktop()->height() - tab_win_->height()) /2);
}

ChatDlgManager::~ChatDlgManager()
{
    clean();

    if ( tab_win_ )
    {
        tab_win_->close();
        delete tab_win_;
        tab_win_ = NULL;
    }

    instance_ = NULL;
}

void ChatDlgManager::initConnections()
{
    MainWindow *main_win = Qtqq::instance()->mainWin();
    connect(main_win->friendView(), SIGNAL(indexContextMenu(RosterIndex *, Menu *)), this, SLOT(onIndexContextMenu(RosterIndex *, Menu *)));
    connect(main_win->groupView(), SIGNAL(indexContextMenu(RosterIndex *, Menu *)), this, SLOT(onIndexContextMenu(RosterIndex *, Menu *)));
    connect(main_win->recentView(), SIGNAL(indexContextMenu(RosterIndex *, Menu *)), this, SLOT(onIndexContextMenu(RosterIndex *, Menu *)));
}

void ChatDlgManager::onIndexContextMenu(RosterIndex *index, Menu *menu)
{
    if ( !menu )
        return;

    if ( index->type() == RIT_Category )
        return;

    RosterView *view = qobject_cast<RosterView *>(sender());
    QAction *begin_chat_act = new QAction(tr("Begin chat"), menu);
    begin_chat_act->setData(QVariant::fromValue<void *>(view)); 
    connect(begin_chat_act, SIGNAL(triggered()), this, SLOT(onBeginChat()));

    menu->addAction(begin_chat_act);
}

void ChatDlgManager::onBeginChat()
{
    QAction *action = qobject_cast<QAction *>(sender());
    RosterView *view = (RosterView *)action->data().value<void *>();

    QModelIndex src_index = view->currentSelectedIndex();
    RosterIndex *roster_index = static_cast<RosterIndex *>(src_index.internalPointer()); 
    if ( roster_index->type() == RIT_Contact )
    {
        openFriendChatDlg(roster_index->id());
    }
    else if ( roster_index->type() == RIT_Group )
    {
        TalkableIndex *t_index = (TalkableIndex *)roster_index;

        openGroupChatDlg(t_index->id(), t_index->gcode());
    }
}

void ChatDlgManager::openFriendChatDlg(const QString &id)
{
    if ( isOpening(id) )
    {
        tab_win_->activatedTab(id);
        tab_win_->activateWindow();
        tab_win_->raise();
        return;
    }

    Roster *roster = Roster::instance();
    Contact *contact = roster->contact(id);
    if ( !contact )
        contact = StrangerManager::instance()->stranger(id);

    QQChatDlg *dlg = NULL;
    dlg= new FriendChatDlg(contact);

    connect(dlg, SIGNAL(chatFinish(QQChatDlg*)), this, SLOT(closeChatDlg(QQChatDlg*)));
    if ( main_win_->recentModel() )
        connect(dlg, SIGNAL(sigMsgSended(QString)), main_win_->recentModel(), SLOT(slotMsgSended(QString)));

    ChatMsgProcessor::instance()->registerListener(dlg);
    opening_chatdlg_.append(dlg);

    QString dlg_name = contact->name().left(4);
    if ( contact->name().size() > 4 )
        dlg_name += "..";

    tab_win_->addTab(dlg, dlg_name);
    tab_win_->show();
    tab_win_->activateWindow();
    tab_win_->raise();
}

void ChatDlgManager::openGroupChatDlg(QString id, QString gcode)
{
    if ( isOpening(id) )
    {
        tab_win_->activatedTab(id);
        tab_win_->activateWindow();
        tab_win_->raise();
        return;
    }

    Roster *roster = Roster::instance();
    Group *group = roster->group(id);

    QQChatDlg *dlg = NULL;
    dlg = new GroupChatDlg(group);

    connect(dlg, SIGNAL(chatFinish(QQChatDlg*)), this, SLOT(closeChatDlg(QQChatDlg*)));

    if ( main_win_->recentModel() )
        connect(dlg, SIGNAL(sigMsgSended(QString)), main_win_->recentModel(), SLOT(slotMsgSended(QString)));

    ChatMsgProcessor::instance()->registerListener(dlg);

    opening_chatdlg_.append(dlg);

    QString dlg_name = group->name().left(4);
    if ( group->name().size() > 4 )
        dlg_name += "..";

    tab_win_->addTab(dlg, dlg_name);
    tab_win_->show();
    tab_win_->activateWindow();
    tab_win_->raise();
}

void ChatDlgManager::openSessChatDlg(const QString &id, const QString &gid) 
{
    if ( isOpening(id) )
    {
        tab_win_->activatedTab(id);
        tab_win_->activateWindow();
        tab_win_->raise();
        return; 
    }

    Group *group = Roster::instance()->group(gid);
    Contact *contact = NULL;
    assert(group);
    if ( group->memberCount() == 0 )
    {
        Roster *roster = Roster::instance();
        contact = StrangerManager::instance()->stranger(id);
    }
    else
    {
        contact = group->member(id)->clone();;
    }

    QQChatDlg *dlg = NULL;
    dlg = new SessChatDlg(contact, group);
    connect(dlg, SIGNAL(chatFinish(QQChatDlg*)), this, SLOT(closeChatDlg(QQChatDlg*)));
    if ( main_win_->recentModel() )
        connect(dlg, SIGNAL(sigMsgSended(QString)), main_win_->recentModel(), SLOT(slotMsgSended(QString)));

    ChatMsgProcessor::instance()->registerListener(dlg);
    opening_chatdlg_.append(dlg);

    QString dlg_name = contact->name().left(4);
    if ( contact->name().size() > 4 )
        dlg_name += "..";

    tab_win_->addTab(dlg, dlg_name);
    tab_win_->show();
    tab_win_->activateWindow();
    tab_win_->raise();
}

bool ChatDlgManager::isOpening(const QString &id) const
{
    foreach(QQChatDlg *chatdlg, opening_chatdlg_)
    {
        if (chatdlg->id() == id)
            return true;
    }
    return false;
}

QQChatDlg* ChatDlgManager::chatDlgById(QString id) const
{
    foreach ( QQChatDlg *dlg, opening_chatdlg_ )
    {
        if ( dlg->id() == id )
            return dlg;
    }
    return NULL;
}

void ChatDlgManager::closeChatDlg(QQChatDlg *listener)
{
    opening_chatdlg_.remove(opening_chatdlg_.indexOf(listener));
    ChatMsgProcessor::instance()->removeListener(listener);
    listener->deleteLater();
}

void ChatDlgManager::notifyDlgById(QString id)
{
    QQChatDlg *dlg = chatDlgById(id);
    if ( dlg )
        tab_win_->blink(dlg);
}

void ChatDlgManager::setMainWin(MainWindow *main_win)
{
    main_win_ = main_win;
}

QQChatDlg *ChatDlgManager::currentChatdlg() const
{
    return tab_win_->currentChatdlg();
}

void ChatDlgManager::clean()
{
    foreach ( QQChatDlg *dlg, opening_chatdlg_ )
    {
        dlg->close();
        delete dlg;
        dlg = NULL;
    }

    opening_chatdlg_.clear();


    MainWindow *main_win = Qtqq::instance()->mainWin();
    disconnect(main_win->friendView(), SIGNAL(indexContextMenu(RosterIndex *, Menu *)), this, SLOT(onIndexContextMenu(RosterIndex *, Menu *)));
    disconnect(main_win->groupView(), SIGNAL(indexContextMenu(RosterIndex *, Menu *)), this, SLOT(onIndexContextMenu(RosterIndex *, Menu *)));
    disconnect(main_win->recentView(), SIGNAL(indexContextMenu(RosterIndex *, Menu *)), this, SLOT(onIndexContextMenu(RosterIndex *, Menu *)));
}


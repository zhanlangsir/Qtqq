#include "groupchatdlg.h"
#include "ui_groupchatdlg.h"

#include <QDateTime>
#include <QList>
#include <QTcpSocket>
#include <QDebug>

#include "json/json.h"

#include "chatwidget/chatdlg_manager.h"
#include "core/groupchatlog.h"
#include "protocol/qq_protocol.h"
#include "roster/group_presister.h"
#include "rostermodel/contact_proxy_model.h"
#include "rostermodel/contact_searcher.h"
#include "rostermodel/roster_index.h"
#include "rostermodel/roster_model.h"
#include "roster/roster.h"
#include "skinengine/qqskinengine.h"

GroupChatDlg::GroupChatDlg(Group *group, ChatDlgType type, QWidget *parent) :
    QQChatDlg(group, type, parent),
    ui(new Ui::GroupChatDlg())
{
    ui->setupUi(this);

    model_  = new RosterModel();
	proxy_model_ = new ContactProxyModel(this);
	proxy_model_->setSourceModel(model_);
	model_->setProxyModel(proxy_model_);

    initUi();  
    updateSkin();
    initConnections();

    setupMemberList();

    searcher_ = new ContactSearcher(this);
	searcher_->initialize(((Group *)talkable_)->members());

    ui->member_view->setModel(proxy_model_);
}

GroupChatDlg::~GroupChatDlg()
{
    if ( model_ )
    {
        delete model_;
        model_ = NULL;
    }

    disconnect();
    delete ui;
}

void GroupChatDlg::setupMemberList()
{
    Group *group = static_cast<Group *>(talkable_);

    if ( group->memberCount() == 0 )
    {
        Protocol::QQProtocol *protocol = Protocol::QQProtocol::instance();
        protocol->requestGroupMemberList((Group *)talkable_);
    }
    else
    {
        QVector<Contact *> members = ((Group *)talkable_)->members();
        foreach ( Contact *contact, members )
        {
            model_->addContactItem(contact);
            replaceUnconverId(contact);
        }
    }
}

void GroupChatDlg::initUi()
{
    setWindowTitle(talkable_->name());
    ui->member_view->setSortingEnabled(true);
    ui->lbl_name_->setText(talkable_->name());
    ui->announcement->setPlainText(((Group *)talkable_)->announcement());

    model_->setIconSize(QSize(25, 25));

    QPixmap pix = talkable_->avatar();
    if ( !pix.isNull() )
        ui->lbl_avatar_->setPixmap(pix);
    else
        ui->lbl_avatar_->setPixmap(QPixmap(QQSkinEngine::instance()->skinRes("default_group_avatar")));

    ui->btn_send_key->setMenu(send_type_menu_);

    ui->splitter_left_->insertWidget(0, &msgbrowse_);
    ui->splitter_left_->setChildrenCollapsible(false);
    ui->v_layout_left_->insertWidget(1, &te_input_);
    ui->splitter_main->setChildrenCollapsible(false);
    ui->splitter_right->setChildrenCollapsible(false);

    //设置分割器大小
    QList<int> main_sizes;
    main_sizes.append(500);
    main_sizes.append(ui->splitter_right->midLineWidth());
    ui->splitter_main->setSizes(main_sizes);

    QList<int> left_sizes;
    left_sizes.append(500);
    left_sizes.append(ui->splitter_left_->midLineWidth());
    ui->splitter_left_->setSizes(left_sizes);

    QList<int> right_sizes;
    right_sizes.append(200);
    right_sizes.append(this->height());
    ui->splitter_right->setSizes(right_sizes);

    this->resize(600, 500);
    te_input_.setFocus();
}

void GroupChatDlg::initConnections()
{
    connect(talkable_, SIGNAL(dataChanged(QVariant, TalkableDataRole)), this, SLOT(onTalkableDataChanged(QVariant, TalkableDataRole)));

    Group *group = static_cast<Group *>(talkable_);
    connect(group, SIGNAL(memberDataChanged(Contact *, TalkableDataRole)), this, SLOT(onGroupMemberDataChanged(Contact *, TalkableDataRole)));
    connect(group, SIGNAL(memberAdded(Contact *)), this, SLOT(onMemberAdded(Contact *)));
    connect(group, SIGNAL(memberRemoved(Contact *)), this, SLOT(onMemberRemoved(Contact *)));

    connect(ui->btn_send_img, SIGNAL(clicked(bool)), this, SLOT(openPathDialog(bool)));
    connect(ui->btn_send_msg, SIGNAL(clicked()), this, SLOT(sendMsg())); connect(ui->btn_qqface, SIGNAL(clicked()), this, SLOT(openQQFacePanel()));
    connect(ui->btn_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->btn_chat_log, SIGNAL(clicked()), this, SLOT(openChatLogWin()));
    connect(ui->member_view, SIGNAL(doubleClicked(const QModelIndex &)), model_, SLOT(onDoubleClicked(const QModelIndex &)));
    connect(model_, SIGNAL(sigDoubleClicked(const RosterIndex *)), this, SLOT(onDoubleClicked(const RosterIndex *)));

    connect(&msgbrowse_, SIGNAL(linkClicked(const QUrl &)), this, SLOT(onLinkClicked(const QUrl &)));

	connect(ui->member_searcher, SIGNAL(textChanged(const QString &)), this, SLOT(onSearch(const QString &)));
}

void GroupChatDlg::updateSkin()
{

}

void GroupChatDlg::onDoubleClicked(const RosterIndex *index)
{
    QString id = index->id();
    openSessOrFriendChatDlg(id);
}

void GroupChatDlg::onLinkClicked(const QUrl &url)
{
    QRegExp sender_reg("\\[(.*)\\]");
    if ( sender_reg.indexIn(url.toString()) != -1 )
    {
        openSessOrFriendChatDlg(sender_reg.cap(1));
    }
}

void GroupChatDlg::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    QQChatDlg::closeEvent(event);

    GroupPresister::instance()->setActivateFlag(talkable_->id());
}

QQChatLog *GroupChatDlg::getChatlog() const
{
    return new GroupChatLog(talkable_->gcode());
}

void GroupChatDlg::onMemberAdded(Contact *contact)
{
    model_->addContactItem(contact);

    searcher_->appendSpell(contact);
    replaceUnconverId(contact);
}

void GroupChatDlg::onMemberRemoved(Contact *contact)
{
    //searcher_->removeSpell(contact);
}

void GroupChatDlg::replaceUnconverId(Contact *contact)
{
    if ( unconvert_ids_.indexOf(contact->id()) != -1 )
    {
        msgbrowse_.replaceIdToName(contact->id(), contact->markname());
    }
}

Contact *GroupChatDlg::findContactById(QString id) const
{
    return ((Group *)talkable_)->member(id);
}

void GroupChatDlg::onTalkableDataChanged(QVariant data, TalkableDataRole role)
{
    Talkable *talkable = qobject_cast<Talkable *>(sender());
    switch ( role )
    {
        case TDR_Avatar:
			ui->lbl_avatar_->setPixmap(talkable->avatar());
            break;
        case TDR_Announcement:
            ui->announcement->setPlainText(data.toString());
            break;
        default:
            break;
    }
}

void GroupChatDlg::onGroupMemberDataChanged(Contact *member, TalkableDataRole role)
{
    model_->talkableDataChanged(member->id(), member->avatar(), role);
}

Contact *GroupChatDlg::getSender(const QString &id) const
{
    return findContactById(id);
}

void GroupChatDlg::onSearch(const QString &str)
{
    if ( str.isEmpty() )
        proxy_model_->endFilter();
    else
    {
        QVector<QString> result;
		searcher_->search(str, result);

        proxy_model_->setFilter(result);
    }
}

void GroupChatDlg::openSessOrFriendChatDlg(QString id)
{
    Roster *roster = Roster::instance();
    Contact *contact = roster->contact(id);
    if ( contact )
        ChatDlgManager::instance()->openFriendChatDlg(id);
    else if ( ((Group *)talkable_)->member(id) )
    {
        ChatDlgManager::instance()->openSessChatDlg(id, talkable_->id());
    }
}

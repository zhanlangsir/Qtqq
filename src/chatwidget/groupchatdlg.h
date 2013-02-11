#ifndef GROUPCHATDLG_H
#define GROUPCHATDLG_H

#include <QVector>

#include "qqchatdlg.h"
#include "core/talkable.h"

namespace Ui
{
    class GroupChatDlg;
}

class QWidget;
class QModelIndex;

class Contact;
class RosterModel;
class ContactSearcher;
class ContactProxyModel;

class GroupChatDlg : public QQChatDlg
{
    Q_OBJECT
public:
    GroupChatDlg(Group *group, ChatDlgType type = kGroup, QWidget *parent = 0);
    ~GroupChatDlg();

public:
    void updateSkin();

    QString code() const
    { return talkable_->gcode(); }
    QString msgSig() const
    { return msg_sig_; }
    const RosterModel* model() const
    { return model_; }

	static QString getMsgSig(QString gid,  QString to_id);

protected:
    virtual void closeEvent(QCloseEvent *event);

private slots:
    void openChatDlgByDoubleClicked(const QModelIndex &index);
	void onTalkableDataChanged(QVariant data, TalkableDataRole role);
    void onGroupMemberDataChanged(Contact *member, TalkableDataRole role);
    void onMemberAdded(Contact *contact);
    void onMemberRemoved(Contact *contact);
    void onSearch(const QString &str);
    void onLinkClicked(const QUrl &url);

private:
    virtual ImgLoader* getImgLoader() const;
    virtual QQChatLog *getChatlog() const;
    virtual Contact *getSender(const QString &id) const;

    void initUi();
    void initConnections();
    void setupMemberList();
    void openSessOrFriendChatDlg(QString id);

    Contact *findContactById(QString id) const;

    void replaceUnconverId(Contact *contact);

private:
    Ui::GroupChatDlg *ui;
    QString msg_sig_;
    QString connection_name_;

    ContactSearcher *searcher_;

	RosterModel *model_;
    ContactProxyModel *proxy_model_;
};

#endif //GROUPCHATDLG_H

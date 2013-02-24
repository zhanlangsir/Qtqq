#ifndef FRIENDCHATDLG_H
#define FRIENDCHATDLG_H

#include <QVariant>
#include <QUrl>

#include "qqchatdlg.h"
#include "core/talkable.h"

namespace Ui
{
    class FriendChatDlg;
}

class Contact;

class FriendChatDlg : public QQChatDlg
{
    Q_OBJECT
public:
    explicit FriendChatDlg(Contact *contact, ChatDlgType type = kFriend, QWidget *parent = 0);
    ~FriendChatDlg();

public:
    void updateSkin();

protected:
    virtual void showOtherMsg(ShareQQMsgPtr msg);

private slots:
	void onTalkableDataChanged(QVariant data, TalkableDataRole role);
    void onOffFileMsg(ShareQQMsgPtr msg);
    void onLinkClicked(const QUrl &url);

    void onOfffileSend();
    void onFileSend();

private:
    virtual QQChatLog *getChatlog() const;
    virtual Contact *getSender(const QString &id) const;

    void initUi();
    void initConnections();
    void getSingleLongNick(QString id);

private:
    Ui::FriendChatDlg *ui;
};

#endif //FRIENDCHATDLG_H

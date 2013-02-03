#ifndef QTQQ_FRIENDCHATDLG_H
#define QTQQ_FRIENDCHATDLG_H

#include <QVariant>

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

private:
    virtual QQChatLog *getChatlog() const;
    virtual Contact *getSender(const QString &id) const;

    void initUi();
    void initConnections();
    void getSingleLongNick(QString id);

private slots:
	void onTalkableDataChanged(QVariant data, TalkableDataRole role);

private:
    Ui::FriendChatDlg *ui;
};

#endif //QTQQ_FRIENDCHATDLG_H

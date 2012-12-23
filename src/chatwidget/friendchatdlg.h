#ifndef QTQQ_FRIENDCHATDLG_H
#define QTQQ_FRIENDCHATDLG_H

#include <QDateTime>
#include <QVariant>

#include "core/qqmsg.h"
#include "core/nameconvertor.h"
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
    virtual ImgSender* getImgSender() const;
    virtual QQChatLog *getChatlog() const;
    virtual void getInfoById(QString id, QString &name, QString &avatar_path, bool &ok) const;

    void initUi();
    void initConnections();
    void getSingleLongNick(QString id);

private slots:
	void onTalkableDataChanged(QVariant data, TalkableDataRole role);

private:
    Ui::FriendChatDlg *ui;
};

#endif //QTQQ_FRIENDCHATDLG_H

#pragma once

#include <QDateTime>

#include "core/qqmsg.h"
#include "core/nameconvertor.h"
#include "qqchatdlg.h"

namespace Ui
{
    class QQFriendChatDlg;
}

class QMouseEvent;

class QQFriendChatDlg : public QQChatDlg
{
    Q_OBJECT
public:
    QQFriendChatDlg(QString uin, QString from_name, FriendInfo curr_user_info, QString avatar_path, QWidget *parent = 0);
    ~QQFriendChatDlg();

public:
    void updateSkin();

private:
    QString converToJson(const QString &raw_msg);
    ImgSender* getImgSender() const;
    QQChatLog *getChatlog() const;

private:
    Ui::QQFriendChatDlg *ui;
    QPoint distance_pos_;
};

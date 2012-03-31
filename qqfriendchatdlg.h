#pragma once

#include <QDateTime>

#include "qqmsg.h"
#include "nameconvertor.h"
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
    QQFriendChatDlg(QString uin, QString from_name, FriendInfo curr_user_info);
    ~QQFriendChatDlg();

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

private:
    QString converToJson(const QString &raw_msg);
    ImgSender* createImgSender();

private:
    Ui::QQFriendChatDlg *ui;
    QPoint distance_pos_;
};

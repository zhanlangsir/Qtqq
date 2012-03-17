#pragma once

#include <QDateTime>

#include "qqmsg.h"
#include "nameconvertor.h"
#include "qqchatdlg.h"

namespace Ui
{
    class QQGroupChatDlg;
}

class QQGroupChatDlg : public QQChatDlg
{
    Q_OBJECT
public:
    QQGroupChatDlg(QString gid, QString name, QString group_code, FriendInfo curr_user_info, CaptchaInfo cap_info);
    ~QQGroupChatDlg();

private:
    QString converToJson(const QString &raw_msg);
    ImgSender* createImgSender();
    void getGfaceSig();
    void getGroupMemberList();

private:
    Ui::QQGroupChatDlg *ui;
    QString group_code_;
    QString gface_key_;
    QString gface_sig_;
};

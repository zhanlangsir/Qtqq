#pragma once

#include <QHttp>

#include "qqmsg.h"
#include "nameconvertor.h"
#include "qqchatdlg.h"

namespace Ui
{
    class QQGroupChatDlg;
}

class QMouseEvent;

class QQGroupChatDlg : public QQChatDlg
{
    Q_OBJECT
public:
    QQGroupChatDlg(QString gid, QString name, QString group_code, FriendInfo curr_user_info, CaptchaInfo cap_info);
    ~QQGroupChatDlg();

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

private slots:
    void getGfaceSigDone(bool err);
    void getGroupMemberListDone(bool err);

private:
    QString converToJson(const QString &raw_msg);
    ImgSender* createImgSender();
    void getGfaceSig();
    void getGroupMemberList();

private:
    Ui::QQGroupChatDlg *ui;
    QHttp http_;
    QString group_code_;
    QString gface_key_;
    QString gface_sig_;
    QPoint distance_pos_;
};

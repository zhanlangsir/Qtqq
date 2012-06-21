#ifndef QTQQ_FRIENDCHATDLG_H
#define QTQQ_FRIENDCHATDLG_H

#include <QDateTime>

#include "core/qqmsg.h"
#include "core/nameconvertor.h"
#include "qqchatdlg.h"

namespace Ui
{
    class FriendChatDlg;
}

class QMouseEvent;

class FriendChatDlg : public QQChatDlg
{
    Q_OBJECT
public:
    FriendChatDlg(QString uin, QString from_name, QString avatar_path, QWidget *parent = 0);
    ~FriendChatDlg();

public:
    void updateSkin();

private:
    QString converToJson(const QString &raw_msg);
    ImgSender* getImgSender() const;
    QQChatLog *getChatlog() const;
    void getInfoById(QString id, QString &name, QString &avatar_path, bool &ok) const;

private:
    Ui::FriendChatDlg *ui;
    QString avatar_path_;
};

#endif //QTQQ_FRIENDCHATDLG_H

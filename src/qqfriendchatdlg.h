#ifndef QTQQ_QQFRIENDCHATDLG_H
#define QTQQ_QQFRIENDCHATDLG_H

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
    QQFriendChatDlg(QString uin, QString from_name, QString avatar_path, QWidget *parent = 0);
    ~QQFriendChatDlg();

public:
    void updateSkin();

private:
    QString converToJson(const QString &raw_msg);
    ImgSender* getImgSender() const;
    QQChatLog *getChatlog() const;
    void getInfoById(QString id, QString &name, QString &avatar_path, bool &ok) const;
    void getSingleLongNick(QString id);

private:
    Ui::QQFriendChatDlg *ui;
    QString avatar_path_;
};

#endif //QTQQ_QQFRIENDCHATDLG_H

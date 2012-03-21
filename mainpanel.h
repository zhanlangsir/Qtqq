#ifndef MAINPANEL_H
#define MAINPANEL_H

#include "qqmsgtip.h"
#include "qqmsgcenter.h"
#include "qqitem.h"
#include "nameconvertor.h"
#include "types.h"
#include "qqpollthread.h"
#include "qqparsethread.h"

#include <QWidget>
#include <QMutex>
#include <QQueue>
#include <QMap>
#include <QVector>

class QHttp;
class QSemaphore;

namespace Ui
{
    class QQMainPanel;
}

class QQMainPanel : public QWidget
{
    Q_OBJECT

public:
    explicit QQMainPanel(CaptchaInfo cap_info, FriendInfo user_info, QWidget *parent = NULL);
    ~QQMainPanel();

public:
    void initialize();

private slots:
    void getFriendListDone(bool err);
    void getGroupListDone(bool err);
    void getOnlineBuddyDone(bool err);
    void getPersonalFaceDone(bool err);
    void getPersonalInfoDone(bool err);
    void openChatDlg(QQMsg::MsgType type, QString gid);
    void closeChatDlg(QQMsgListener *listener);
    void openChatDlgByDoubleClick(const QModelIndex& index);
    void changeFriendStatus(QString id, FriendStatus state);

private:
    QQItem *findQQItemById(QString id);
    void getFriendList();
    void getGroupList();
    void getOnlineBuddy();
    void getPersonalFace();
    void getPersonalInfo();
    void parseFriendsInfo(const QByteArray &array, QQItem *root_item);
    void parseGroupsInfo(const QByteArray &array, QQItem *root_item);

private:
    Ui::QQMainPanel *ui;
    QHttp *main_http_;
    CaptchaInfo cap_info_;
    FriendInfo curr_user_info_;
    QSemaphore *poll_semapore_;
    QSemaphore *parse_semapore_;
    QQPollThread *poll_thread_;
    QQParseThread *parse_thread_;
    QQueue<QByteArray> *message_queue_;
    QMutex *check_old_msg_lock;
    QQMsgTip *msg_tip_;
    QQMsgCenter *msg_center_;

    NameConvertor convertor_;
    QVector<GroupInfo*> groups_info_;
    QVector<QQItem*> friends_info_;
};

#endif // MAINPANEL_H

#pragma once

#include <QWidget>
#include <QMutex>
#include <QCloseEvent>
#include <QQueue>
#include <QMap>
#include <QVector>
#include <QSystemTrayIcon>
#include <QMenu>

#include "qqmsgtip.h"
#include "qqmsgcenter.h"
#include "qqitem.h"
#include "nameconvertor.h"
#include "types.h"
#include "qqpollthread.h"
#include "qqchatdlg.h"
#include "qqparsethread.h"
#include "qqsystemtray.h"

class QHttp;
class QSemaphore;
class QHttpRequestHeader;
class QQItemModel;

namespace Ui
{
    class QQMainPanel;
}

class QQMainPanel : public QWidget
{
    Q_OBJECT

public:
    explicit QQMainPanel(FriendInfo user_info, QWidget *parent = NULL);
    ~QQMainPanel();

public:
    void initialize();

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void changeUserStatus(int idx);
    void getFriendListDone(bool err);
    void getGroupListDone(bool err);
    void getOnlineBuddyDone(bool err);
    void getPersonalFaceDone(bool err);
    void getPersonalInfoDone(bool err);
    void getRecentListDone(bool err);
    void getSingleLongNickDone(bool err);
    void openChatDlg(QQMsg::MsgType type, QString gid, QString gcode);
    void closeChatDlg(QQChatDlg *listener);
    void openChatDlgByDoubleClick(const QModelIndex& index);
    void openFriendRequestDlg(QQMsg *msg);
    void openGroupRequestDlg(QQMsg *msg);
    void changeFriendStatus(QString id, FriendStatus status, ClientType client_type);
    void changeRecentList(const QQChatMsg *msg);
    void changeRecentList(const QQGroupChatMsg *msg);
    void trayActivated(QSystemTrayIcon::ActivationReason reason);
    void openMainMenu();
    void setMute(bool mute);

private:
    QQItem* findFriendItemById(QString id);
    QQItem* findGroupItemById(QString id);
    QQItem* findRecentListItemById(QString id);
    int getNewPosition(const QQItem *item) const;
    void getFriendList();
    void getGroupList();
    void getOnlineBuddy();
    void getPersonalFace();
    void getPersonalInfo();
    void getRecentList();
    void getSingleLongNick();
    int getStatusIndex(FriendStatus status);
    void parseFriendsInfo(const QByteArray &array, QQItem *const root_item);
    void parseGroupsInfo(const QByteArray &array, QQItem *const root_item);
    void parseRecentList(const QByteArray &array, QQItem *const root_item);
    void createTray();
    void createActions();
    void setupStatus();
    QString getStatusByIndex(int idx) const;

private:
    Ui::QQMainPanel *ui;
    QHttp *main_http_;
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
    //QVector<QQItem*> groups_info_;
    //QVector<QQItem*> friends_info_;
    QQItemModel *friend_model_;
    QQItemModel *group_model_;
    QVector<QQItem*> recents_info_;
    QVector<QQChatDlg*> opening_chatdlg_;
    QQItem *recent_list_root_;

    //system tray
    QQSystemTray *trayIcon;
    QAction *minimizeAction;
    QAction *restoreAction;
    QAction *quitAction;
    QMenu *trayIconMenu;

    QMenu *main_menu_;
    QAction *act_mute_;
};

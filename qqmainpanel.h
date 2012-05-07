#ifndef QTQQ_MAINPANEL_H
#define QTQQ_MAINPANEL_H

#include <QWidget>
#include <QCloseEvent>
#include <QQueue>
#include <QMap>
#include <QVector>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QModelIndex>

#include "nameconvertor.h"
#include "qqsystemtray.h"
#include "qqmsg.h"
#include "recentlistitemmodel.h"

class QHttp;
class QHttpRequestHeader;

class QQItemModel;
class QQParseThread;
class QQPollThread;
class QQChatDlg;
class QQItem;
class QQMsgTip;
class QQMsgCenter;
class QQLoginCore;

namespace Ui
{
    class QQMainPanel;
}

class QxtGlobalShortcut;

class QQMainPanel : public QWidget
{
    Q_OBJECT
signals:
    void sig_logout();

public:
    explicit QQMainPanel(FriendInfo user_info, QWidget *parent = NULL);
    ~QQMainPanel();

public:
    void initialize();

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void changeMyStatus(int idx);
    void getFriendListDone(bool err);
    void getGroupListDone(bool err);
    void getOnlineBuddyDone(bool err);
    void getRecentListDone(bool err);
    void openChatDlg(QQMsg::MsgType type, QString gid, QString gcode);
    void closeChatDlg(QQChatDlg *listener);
    void openChatDlgByDoubleClick(const QModelIndex& index);
    void openFriendRequestDlg(ShareQQMsgPtr msg);
    void openGroupRequestDlg(ShareQQMsgPtr msg);
    void changeFriendStatus(QString id, FriendStatus status, ClientType client_type);
    void trayActivated(QSystemTrayIcon::ActivationReason reason);
    void openMainMenu();
    void setMute(bool mute);
    void slot_logout();
    void openFirstChatDlg();

private:
    QQItem* findFriendItemById(QString id);
    QQItem* findGroupItemById(QString id);
    QQItem* findRecentListItemById(QString id);
    void getFriendList();
    void getGroupList();
    void getOnlineBuddy();
    void getPersonalFace();
    void getPersonalInfo();
    void getRecentList();
    void getSingleLongNick();
    int getStatusIndex(FriendStatus status);
    void createTray();
    void createActions();
    void setupLoginStatus();
    QString getStatusByIndex(int idx) const;

private:
    QQLoginCore *login_;
    Ui::QQMainPanel *ui;
    QHttp *main_http_;
    FriendInfo curr_user_info_;
    QQPollThread *poll_thread_;
    QQParseThread *parse_thread_;
    QQueue<QByteArray> *message_queue_;
    QQMsgTip *msg_tip_;
    QQMsgCenter *msg_center_;

    NameConvertor convertor_;
    FriendItemModel *friend_model_;
    GroupItemModel *group_model_;
    RecentListItemModel *recent_model_;
    QVector<QQChatDlg*> opening_chatdlg_;

    //system tray
    QQSystemTray *trayIcon;
    QAction *minimizeAction;
    QAction *restoreAction;
    QAction *quitAction;
    QAction *act_logout_;
    QMenu *trayIconMenu;

    QMenu *main_menu_;
    QAction *act_mute_;

    QxtGlobalShortcut *open_chat_dlg_sc_;
};

#endif //QTQQ_MAINPANEL_H

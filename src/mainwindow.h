#ifndef QTQQ_MAINPANEL_H
#define QTQQ_MAINPANEL_H

#include <QCloseEvent>
#include <QQueue>
#include <QMap>
#include <QVector>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QModelIndex>
#include <QPoint>

#include "log4qt/log4qt.h"
#include "core/nameconvertor.h"
#include "core/qqmsg.h"
#include "qqsystemtray.h"
#include "recentlistitemmodel.h"

class QHttp;
class QHttpRequestHeader;

class QQItemModel;
class ParseThread;
class PollThread;
class QQChatDlg;
class QQItem;
class MsgTip;
class QQMsgCenter;
class QQLoginCore;

namespace Ui
{
    class MainWindow;
}

class QxtGlobalShortcut;

class MainWindow : public QWidget
{
    Q_OBJECT
signals:
    void sig_logout();

public:
    explicit MainWindow(QWidget *parent = NULL);
    ~MainWindow();

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
    Ui::MainWindow *ui;
    QHttp *main_http_;
    PollThread *poll_thread_;
    ParseThread *parse_thread_;
    QQueue<QByteArray> *message_queue_;
    MsgTip *msg_tip_;
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

    QPoint distance_pos_;

    QPoint old_pos_;
    bool is_mouse_down_;
    bool left_,right_,bottom_;
};

#endif //QTQQ_MAINPANEL_H

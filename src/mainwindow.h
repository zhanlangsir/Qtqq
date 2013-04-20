#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QCloseEvent>
#include <QSystemTrayIcon>

#include "utils/contact_status.h"

namespace Ui
{
    class MainWindow;
}

class QHttp;

class QxtGlobalShortcut;

class Menu;
class ContactProxyModel;
class QModelIndex;
class ContactSearcher;
class FriendSearcher;
class QQChatDlg;
class QQLoginCore;
class RecentModel;
class RosterModel;
class PluginManageDlg;
class RosterIndex;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = NULL);
    ~MainWindow();

public:
    void initialize();
    const RecentModel* recentModel() const
    { return recent_model_; }

	// 更新托盘显示的用户信息
	void updateLoginUser() const;
	void clean();

    Menu *mainMenu() const
    { return main_menu_; }

protected:
    virtual void closeEvent(QCloseEvent *);

private slots:
    void onFriendListItemPressed(const QModelIndex &index);
    void changeMyStatus(int idx);
    void getFriendListDone(bool err);
    void getGroupListDone(bool err);
    void getOnlineBuddyDone(bool err);
    void getRecentListDone(bool err);
    void openMainMenu();
    void setMute(bool mute);
    void openFirstChatDlg();
    void onTrayIconClicked(QSystemTrayIcon::ActivationReason reason);
	void onSearch(const QString &str);
    void aboutQt();
    void onMainMenuclicked();
    void snapshot();
    void onBeginChat();

private:
    void getFriendList();
    void getGroupList();
    void getOnlineBuddy();
    void getPersonalFace();
    void getPersonalInfo();
    void getRecentList();
    void getSingleLongNick();
    int getStatusIndex(ContactStatus status);
    void setupLoginStatus();
    QString getStatusByIndex(int idx) const;
    void initHotkey();
    QString hashO(const QString &uin, const QString &ptwebqq);

private:
    Ui::MainWindow *ui;
    QHttp *main_http_;

	RosterModel *contact_model_;
    RosterModel *group_model_;
    RecentModel *recent_model_;

    ContactSearcher *searcher_;
	ContactProxyModel *contact_proxy_model_;

    Menu *main_menu_;
    QAction *act_mute_;

    QAction *begin_chat_;

    QxtGlobalShortcut *open_chat_dlg_sc_;
};

#endif //MAINWINDOW_H

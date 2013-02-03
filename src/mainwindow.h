#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCloseEvent>
#include <QMenu>

#include "core/nameconvertor.h"
#include "core/qqmsg.h"

class QHttp;

class QQChatDlg;
class QQLoginCore;
class FriendSearcher;
class RosterModel;
class RecentModel;

namespace Ui
{
    class MainWindow;
}

class QxtGlobalShortcut;

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

protected:
    virtual void closeEvent(QCloseEvent *);

private slots:
    void changeMyStatus(int idx);
    void getFriendListDone(bool err);
    void getGroupListDone(bool err);
    void getOnlineBuddyDone(bool err);
    void getRecentListDone(bool err);
    void openMainMenu();
    void setMute(bool mute);
    void openFirstChatDlg();

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

private:
    Ui::MainWindow *ui;
    QHttp *main_http_;

    NameConvertor convertor_;
	RosterModel *contact_model_;
    RosterModel *group_model_;
    RecentModel *recent_model_;

    QMenu *main_menu_;
    QAction *act_mute_;

    QxtGlobalShortcut *open_chat_dlg_sc_;
};

#endif //MAINWINDOW_H

#ifndef QQCHATDLG_H
#define QQCHATDLG_H

#include <QVector>

#include "core/qqmsglistener.h"
#include "core/talkable.h"
#include "msgbrowse.h"
#include "qqtextedit.h"
#include "interfaces/iobserver.h"

class QAction;
class QMenu;
class QShortcut;

class QQChatLog;
class QQFacePanel;

class QQChatDlg : public QWidget, public QQMsgListener, public IObserver
{
    Q_OBJECT
signals:
    void chatFinish(QQChatDlg *listener);
    void sigFriendMsgSended(QString id);
    void sigGroupMsgSended(QString id);
    void sigSessMsgSended(QString id);
    void sigMsgSended(QString id);

public:
    enum ChatDlgType {kGroup, kFriend, kSess};
    QQChatDlg(Talkable *talkable, ChatDlgType type, QWidget *parent = 0);
    virtual ~QQChatDlg();

    void showMsg(ShareQQMsgPtr msg);
    virtual void showOldMsg(QVector<ShareQQMsgPtr> msgs);

    virtual void insertImage(const QString &file_path);

    QString id() const
    { return talkable_->id(); }
	Talkable *talkable() const
	{ return talkable_; }

    ChatDlgType type() const
    { return type_; }
    virtual void updateSkin() = 0;

    void installEditorEventFilter(QObject *filter_obj);
    void removeEditorEventFilter(QObject *filter_obj);

protected:
    void closeEvent(QCloseEvent *);
    bool eventFilter(QObject * obj, QEvent * e);
    virtual void showOtherMsg(ShareQQMsgPtr msg);
    virtual void send(const QVector<QQChatItem> &msgs);

protected:
    MsgBrowse msgbrowse_;
	QQTextEdit te_input_;

	Talkable *talkable_;
    QVector<QString> unconvert_ids_;
    QMenu *send_type_menu_;

private slots:
    void openPathDialog(bool);
    void openQQFacePanel();
    void openChatLogWin();
    void sendMsg();
    virtual void onMsgSendDone(bool ok, QString msg);
    void setSendByReturn(bool checked);
    void setSendByCtrlReturn(bool checked);
    void onImageDoubleClicked(QString src);

private:
    virtual QQChatLog *getChatlog() const = 0;

    virtual Contact *getSender(const QString &id) const = 0;
    QString converToShow(const QString &converting_html);
    QString escape(QString raw_html) const;
    virtual void onNotify(Protocol::Event *event);

    QString saveImage(const QString &file, const QByteArray &data);
    QString getImageFormat(const QByteArray &data);

private:    
    QQFacePanel *qqface_panel_;

    ChatDlgType type_;
    
    QAction *act_return_;
    QAction *act_ctrl_return_;
    
    bool send_by_return_;

    QShortcut *sc_close_win_;
};

#endif //QQCHATDLG_H

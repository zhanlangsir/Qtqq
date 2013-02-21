#ifndef QTQQ_QQCHATDLG_H
#define QTQQ_QQCHATDLG_H

#include <QDateTime>
#include <QHash>
#include <QScrollBar>
#include <QTcpSocket>
#include <QVector>
#include <QUuid>

#include "core/qqmsglistener.h"
#include "core/qqchatitem.h"
#include "core/talkable.h"
#include "msgbrowse.h"
#include "qqfacepanel.h"
#include "qqtextedit.h"
#include "interfaces/iobserver.h"

class QAction;
class QMenu;
class QShortcut;

class ImgLoader;
class QQChatLog;

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

    QString id() const
    { return talkable_->id(); }
	Talkable *talkable() const
	{ return talkable_; }

    ChatDlgType type() const
    { return type_; }
    virtual void updateSkin() = 0;

    void sendImage(const QString &file_path, const QByteArray &data);

protected:
    void closeEvent(QCloseEvent *);
    bool eventFilter(QObject * obj, QEvent * e);
    virtual void showOtherMsg(ShareQQMsgPtr msg);

protected:
    MsgBrowse msgbrowse_;
	QQTextEdit te_input_;
    QHash<QString, QString> file_path_;

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
    virtual ImgLoader* getImgLoader() const;

    virtual Contact *getSender(const QString &id) const = 0;
    QString converToShow(const QString &converting_html);
    QString escape(QString raw_html) const;
    virtual void onNotify(Protocol::Event *event);
    QString getUniqueId() const
    {
        return QUuid::createUuid().toString();
    }

    QString saveImage(const QString &file, const QByteArray &data);
    QString getImageFormat(const QByteArray &data);

private:    
    ImgLoader *img_loader_;
    QQFacePanel *qqface_panel_;

    ChatDlgType type_;
    
    QAction *act_return_;
    QAction *act_ctrl_return_;
    
    bool send_by_return_;

    QShortcut *sc_close_win_;
};

#endif //QTQQ_QQCHATDLG_H

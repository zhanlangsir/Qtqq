#ifndef QTQQ_QQCHATDLG_H
#define QTQQ_QQCHATDLG_H

#include <QDateTime>
#include <QHash>
#include <QScrollBar>
#include <QTcpSocket>
#include <QVector>
#include <QUuid>

#include "core/types.h"
#include "core/nameconvertor.h"
#include "core/qqmsglistener.h"
#include "core/msgsender.h"
#include "core/qqchatitem.h"
#include "core/talkable.h"
#include "msgbrowse.h"
#include "qqfacepanel.h"
#include "qqtextedit.h"

class QAction;
class QMenu;
class QShortcut;

class ImgSender;
class ImgLoader;
class QQChatLog;
class QQItem;

class QQChatDlg : public QWidget, public QQMsgListener
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

public:
    void showMsg(ShareQQMsgPtr msg);
    virtual void showOldMsg(QVector<ShareQQMsgPtr> msgs);

    QString id() const
    { return talkable_->id(); }
	Talkable *talkable() const
	{ return talkable_; }

    QString getUniqueId()
    {
        return QUuid::createUuid().toString();
    }

    ChatDlgType type() const
    { return type_; }
    virtual void updateSkin() = 0;

    FileInfo getUploadedFileInfo(QString src)
    {
        return id_file_hash_[src];
    }

protected:
    void closeEvent(QCloseEvent *);
    bool eventFilter(QObject * obj, QEvent * e);

protected:
    MsgBrowse msgbrowse_;
	QQTextEdit te_input_;
    QHash<QString, FileInfo> id_file_hash_;
    QString send_url_;
	int msg_id_;

	Talkable *talkable_;
    QTcpSocket fd_;
    QVector<QString> unconvert_ids_;
    QMenu *send_type_menu_;
    NameConvertor convertor_;

private slots:
    void openPathDialog(bool);
    void openQQFacePanel();
    void openChatLogWin();
    void sendMsg();
    void setFileInfo(QString unique_id, FileInfo file_info);
    virtual void onMsgSendDone(bool ok, QString msg);
    void setSendByReturn(bool checked);
    void setSendByCtrlReturn(bool checked);

private:
	virtual QString chatItemToJson(const QVector<QQChatItem> &items) = 0;
    virtual ImgSender* getImgSender() const = 0;
    virtual QQChatLog *getChatlog() const = 0;
    virtual ImgLoader* getImgLoader() const;

    virtual void getInfoById(QString id, QString &name, QString &avatar_path, bool &ok) const = 0;
    QString converToShow(const QString &converting_html);
    QString escape(QString raw_html) const;

private:    
    ImgSender *img_sender_;
    ImgLoader *img_loader_;
    QQFacePanel *qqface_panel_;
    MsgSender *msg_sender_;

    ChatDlgType type_;
    
    QAction *act_return_;
    QAction *act_ctrl_return_;
    
    bool send_by_return_;

    QShortcut *sc_close_win_;
};

#endif //QTQQ_QQCHATDLG_H

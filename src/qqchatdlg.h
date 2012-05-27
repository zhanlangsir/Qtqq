#ifndef QTQQ_QQCHATDLG_H
#define QTQQ_QQCHATDLG_H

#include <QDialog>
#include <QHash>
#include <QScrollBar>
#include <QDateTime>
#include <QTcpSocket>

#include "qqfacepanel.h"
#include "qqtextedit.h"
#include "core/types.h"
#include "core/nameconvertor.h"
#include "core/qqmsglistener.h"
#include "core/qqmsgsender.h"
#include "qqwidget.h"

class QMenu;
class QAction;

class ImgSender;
class ImgLoader;
class QQChatLog;

class QQChatDlg : public QQWidget, public QQMsgListener
{
    Q_OBJECT
signals:
    void chatFinish(QQChatDlg *listener);

public:
    enum ChatDlgType {kGroup, kFriend};
    QQChatDlg(QString id, QString name, FriendInfo curr_user_info, QWidget *parent = 0);
    virtual ~QQChatDlg();

public:
    void setNameConvertor(NameConvertor convertor)
    { convertor_ = convertor; }
    void showMsg(ShareQQMsgPtr msg);
    void showQQFace(QString face_id);
    void showOldMsg(QVector<ShareQQMsgPtr> msgs);
    QString id() const
    { return id_; }
    QString getUniqueId()
    {
        return QDateTime::currentDateTime().toString("MM-dd-hh-mm-ss");
    }

    void set_type(ChatDlgType type)
    { type_ = type; }
    ChatDlgType type() const
    { return type_; }
    virtual void updateSkin() = 0;


protected:
    void closeEvent(QCloseEvent *);
    bool eventFilter(QObject * obj, QEvent * e);

protected:
    QString send_url_;
    QQTextEdit te_messages_;
    QQTextEdit te_input_;
    QHash<QString, FileInfo> id_file_hash_;

    QString id_;
    int msg_id_;
    QString name_;
    QTcpSocket fd_;
    NameConvertor convertor_;
    QVector<QString> unconvert_ids_;
    QMenu *send_type_menu_;

private slots:
    void openPathDialog(bool);
    void openQQFacePanel();
    void openChatLogWin();
    void sendMsg();
    void setFontStyle(QFont font, QColor color, int size);
    void setFileInfo(QString unique_id, FileInfo file_info);
    void silderToBottom(int min, int max)
    { 
        Q_UNUSED(min);
        te_messages_.verticalScrollBar()->setValue(max); 
    }
    void setSendByReturn(bool checked);
    void setSendByCtrlReturn(bool checked);

private:
    virtual QString converToJson(const QString &raw_msg) = 0;
    virtual ImgSender* createImgSender() = 0;
    virtual QQChatLog *getChatlog() const;
    virtual ImgLoader* getImgLoader() const;

private:    
    FriendInfo curr_user_info_;
    ImgLoader *img_loader_;
    ImgSender *img_sender_;
    QQFacePanel *qqface_panel_;
    QQMsgSender *msg_sender_;

    ChatDlgType type_;
    
    QAction *act_return_;
    QAction *act_ctrl_return_;
    
    bool send_by_return_;
};

#endif //QTQQ_QQCHATDLG_H

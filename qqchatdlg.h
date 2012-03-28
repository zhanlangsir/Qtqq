#pragma once

#include "qqfacepanel.h"
#include "qqtextedit.h"
#include "types.h"
#include "nameconvertor.h"
#include "qqmsglistener.h"
#include "qqmsgsender.h"

#include <QDialog>
#include <QHash>
#include <QScrollBar>
#include <QDateTime>

class QQChatDlg : public QDialog, public QQMsgListener
{
    Q_OBJECT
signals:
    void chatFinish(QQChatDlg *listener);

public:
    enum ChatDlgType {kGroup, kFriend};
    QQChatDlg(QString id, QString name, FriendInfo curr_user_info, CaptchaInfo cap_info, QWidget *parent = 0);
    virtual ~QQChatDlg();

public:
    void setCaptchaInfo(CaptchaInfo cap_info)
    {
        cap_info_ = cap_info;
        te_messages_.setCaptchaInfo(cap_info);
    }
    void setNameConvertor(NameConvertor convertor)
    { convertor_ = convertor; }
    void showMsg(const QQMsg *msg);
    void showQQFace(QString face_id);
    void showOldMsg(QVector<QQMsg*> msgs);
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


protected:
    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *e);

protected:
    QString send_url_;
    QQTextEdit te_messages_;
    QQTextEdit te_input_;
    QHash<QString, FileInfo> id_file_hash_;

    QString id_;
    int msg_id_;
    QString name_;
    CaptchaInfo cap_info_;
    QTcpSocket fd_;
    NameConvertor convertor_;
    QVector<QString> unconvert_ids_;

private slots:
    void openPathDialog(bool);
    void openQQFacePanel();
    void sendMsg();
    void setFontStyle(QFont font, QColor color, int size);
    void setFileInfo(QString unique_id, FileInfo file_info);
    void silderToBottom(int min, int max)
    { 
        Q_UNUSED(min);
        te_messages_.verticalScrollBar()->setValue(max); 
    }

private:
    virtual QString converToJson(const QString &raw_msg) = 0;
    virtual ImgSender* createImgSender() = 0;

private:    
    FriendInfo curr_user_info_;
    ImgLoader *img_loader_;
    ImgSender *img_sender_;
    QQFacePanel *qqface_panel_;
    QQMsgSender *msg_sender_;

    ChatDlgType type_;
};

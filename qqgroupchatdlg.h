#pragma once

#include <QHttp>
#include <QSqlDatabase>

#include "qqmsg.h"
#include "nameconvertor.h"
#include "qqchatdlg.h"

namespace Ui
{
    class QQGroupChatDlg;
}

class QMouseEvent;
class QWidget;
class QQItem;

class QQGroupChatDlg : public QQChatDlg
{
    Q_OBJECT
public:
    QQGroupChatDlg(QString gid, QString name, QString group_code, FriendInfo curr_user_info, QString avatar_path, QWidget *parent = 0);
    ~QQGroupChatDlg();

protected:
    void closeEvent(QCloseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    ImgLoader* getImgLoader() const;

private slots:
    void getGfaceSigDone(bool err);
    void getGroupMemberListDone(bool err);

private:
    QString converToJson(const QString &raw_msg);
    ImgSender* createImgSender();
    void getGfaceSig();
    void getGroupMemberList();
    void createSql();
    void createSigSql();
    void parseGroupMemberList(const QByteArray &array, QQItem *const root_item);
    void readFromSql();
    void readSigFromSql();
    void replaceUnconverId();
    void writeMemberInfoToSql();
    QQItem *findItemById(QString id) const;

private:
    Ui::QQGroupChatDlg *ui;
    QHttp http_;
    QString group_code_;
    QString gface_key_;
    QString gface_sig_;
    QPoint distance_pos_;

    QQItem *member_root_;
    QString connection_name_;
};

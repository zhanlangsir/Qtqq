#ifndef QTQQ_QQGROUPCHATDLG_H
#define QTQQ_QQGROUPCHATDLG_H

#include <QHttp>
#include <QSqlDatabase>

#include "core/qqmsg.h"
#include "core/nameconvertor.h"
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
    QQGroupChatDlg(QString gid, QString name, QString group_code, QString avatar_path, QWidget *parent = 0);
    ~QQGroupChatDlg();

public:
    void updateSkin();

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void getGfaceSigDone(bool err);
    void getGroupMemberListDone(bool err);

private:
    QString converToJson(const QString &raw_msg);
    ImgSender* getImgSender() const;
    ImgLoader* getImgLoader() const;
    QQChatLog *getChatlog() const;
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
    void getInfoById(QString id, QString &name, QString &avatar_path, bool &ok) const;

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

#endif //QTQQ_QQGROUPCHATDLG_H

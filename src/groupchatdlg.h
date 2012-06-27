#ifndef QTQQ_GROUPCHATDLG_H
#define QTQQ_GROUPCHATDLG_H

#include <QHttp>
#include <QSqlDatabase>

#include "core/qqmsg.h"
#include "core/nameconvertor.h"
#include "qqchatdlg.h"

namespace Ui
{
    class GroupChatDlg;
}

class QMouseEvent;
class QWidget;
class QQItem;
class QQItemModel;

class GroupChatDlg : public QQChatDlg
{
    Q_OBJECT
public:
    GroupChatDlg(QString gid, QString name, QString group_code, QString avatar_path, QWidget *parent = 0);
    ~GroupChatDlg();

public:
    void updateSkin();

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void getGfaceSigDone(bool err);
    void getGroupMemberListDone(bool err);

private:
    virtual QString converToJson(const QString &raw_msg);
    virtual ImgSender* getImgSender() const;
    virtual ImgLoader* getImgLoader() const;
    virtual QQChatLog *getChatlog() const;
    virtual void getInfoById(QString id, QString &name, QString &avatar_path, bool &ok) const;

    void initUi();
    void initConnections();

    void getGfaceSig();
    void getGroupMemberList();
    void parseGroupMemberList(const QByteArray &array);

    void createSql();
    void createSigSql();
    void readFromSql();
    void readSigFromSql();
    void writeMemberInfoToSql();

    QQItem *findItemById(QString id) const;

    void replaceUnconverId();

private:
    Ui::GroupChatDlg *ui;
    QHttp http_;
    QString group_code_;
    QString gface_key_;
    QString gface_sig_;
    QPoint distance_pos_;
    QQItemModel *model_;
    QString connection_name_;
    QString avatar_path_;
};

#endif //QTQQ_GROUPCHATDLG_H

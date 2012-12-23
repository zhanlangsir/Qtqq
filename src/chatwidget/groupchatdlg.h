#ifndef QTQQ_GROUPCHATDLG_H
#define QTQQ_GROUPCHATDLG_H

#include <QHttp>
#include <QSqlDatabase>

#include "core/qqmsg.h"
#include "core/nameconvertor.h"
#include "qqchatdlg.h"
#include "core/talkable.h"

namespace Ui
{
    class GroupChatDlg;
}

class QMouseEvent;
class QWidget;
class QQItem;
class QQItemModel;
class QModelIndex;

class GroupChatDlg : public QQChatDlg
{
    Q_OBJECT
public:
    GroupChatDlg(Group *group, ChatDlgType type = kGroup, QWidget *parent = 0);
    ~GroupChatDlg();

public:
    void updateSkin();

    QString code() const
    { return talkable_->gcode(); }
    QString key() const
    { return gface_key_; }
    QString sig() const
    { return gface_sig_; }
    QString msgSig() const
    { return msg_sig_; }
    const QQItemModel* model() const
    { return model_; }

	static QString getMsgSig(QString gid,  QString to_id);

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void getGfaceSigDone(bool err);
    void getGroupMemberListDone(bool err);
    void openSessOrFriendChatDlg(QString id);
    void openChatDlgByDoubleClicked(const QModelIndex &index);
	void onTalkableDataChanged(QVariant data, TalkableDataRole role);

private:
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
    QString gface_key_;
    QString gface_sig_;
    QString msg_sig_;
    QPoint distance_pos_;
    QString connection_name_;

	QQItemModel *model_;
};

#endif //QTQQ_GROUPCHATDLG_H

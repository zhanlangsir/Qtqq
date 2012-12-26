#ifndef QTQQ_SESSCHATDLG_H
#define QTQQ_SESSCHATDLG_H

#include "chatwidget/qqchatdlg.h"

namespace Ui
{
    class SessChatDlg;
};

class Group;

class SessChatDlg : public QQChatDlg
{
    Q_OBJECT
public:
    explicit SessChatDlg(Contact *contact, Group *group, ChatDlgType type = kSess, QWidget *parent = 0);
    ~SessChatDlg();

public:
    void updateSkin();

private:
    virtual ImgSender* getImgSender() const;
    virtual QQChatLog *getChatlog() const;
    virtual void getInfoById(QString id, QString &name, QString &avatar_path, bool &ok) const;
	virtual QString chatItemToJson(const QVector<QQChatItem> &items);
    
    void initUi();
    void initConnections();

private:
    Ui::SessChatDlg *ui_;
	Group *group_;
};

#endif // QTQQ_SESSCHATDLG_H

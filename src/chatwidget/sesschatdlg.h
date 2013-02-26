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

    virtual void insertImage(const QString &file_path);

public:
    void updateSkin();

private:
    virtual QQChatLog *getChatlog() const;
    virtual Contact *getSender(const QString &id) const;
    virtual void send(const QVector<QQChatItem> &msgs);
    
    void initUi();
    void initConnections();

private:
    Ui::SessChatDlg *ui_;
	Group *group_;
};

#endif // QTQQ_SESSCHATDLG_H

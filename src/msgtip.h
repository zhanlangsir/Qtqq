#ifndef QTQQ_MSGTIP_H
#define QTQQ_MSGTIP_H

#include <QWidget>
#include <QMutex>
#include <QPoint>

#include "core/soundplayer.h"
#include "core/qqmsg.h"
#include "core/nameconvertor.h"

namespace Ui
{
class MsgTip;
}

class MainWindow;
class QModelIndex;

class MsgTip : public QWidget
{
    Q_OBJECT
public:
    MsgTip(QWidget *parent = 0);

signals:
    void newUncheckMsgArrived();
    void activateFriendRequestDlg(ShareQQMsgPtr msg);
    void activateGroupRequestDlg(ShareQQMsgPtr msg);
    void noUncheckMsg();

public slots:
    void slotActivated(const QModelIndex &index);
    void pushMsg(ShareQQMsgPtr msg);

public:
    void addItem(ShareQQMsgPtr msg);
    void removeItem(QString id);
    void setConvertor(NameConvertor *convertor)
    { convertor_ = convertor; }
    void setMainWindow(const MainWindow *main_win)
    { main_win_ = main_win; }
    void activatedChat(int i);

    void show(QPoint pos);

protected:
    void leaveEvent(QEvent *);

private:
    void bibibi(SoundPlayer::SoundType type);
    bool getStrangerInfo(QString id, QString gid, QString &name, QString &token) const;

private:
    Ui::MsgTip *ui;
    QMutex lock;
    NameConvertor *convertor_;
    const MainWindow *main_win_;
};

#endif //QTQQ_MSGTIP_H

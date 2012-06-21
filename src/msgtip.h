#ifndef QTQQ_MSGTIP_H
#define QTQQ_MSGTIP_H

#include <QWidget>
#include <QComboBox>
#include <QMutex>
#include <QMap>
#include <QPoint>

#include "core/soundplayer.h"
#include "core/qqmsg.h"
#include "core/nameconvertor.h"

namespace Ui
{
class MsgTip;
}

class MsgTip : public QWidget
{
    Q_OBJECT
public:
    MsgTip(QWidget *parent = 0);

signals:
    void addItemDone();
    void activatedChatDlg(QQMsg::MsgType type, QString talk_to, QString gcode);
    void activateFriendRequestDlg(ShareQQMsgPtr msg);
    void activateGroupRequestDlg(ShareQQMsgPtr msg);
    void bibibi(SoundPlayer::SoundType type);

public slots:
    void slotActivated(int index);

public:
    void pushMsg(ShareQQMsgPtr msg);
    void addItem(ShareQQMsgPtr msg);
    void removeItem(QString id);
    void setConvertor(NameConvertor *convertor)
    { convertor_ = convertor; }

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    bool eventFilter(QObject *obj, QEvent *e);

private slots:
    void beginBibibi(SoundPlayer::SoundType type);

private:
    Ui::MsgTip *ui;
    QMutex lock;
    NameConvertor *convertor_;

    QPoint distance_pos_;
};

#endif //QTQQ_MSGTIP_H

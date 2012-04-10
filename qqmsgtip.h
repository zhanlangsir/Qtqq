#pragma once

#include "qqmsg.h"
#include "nameconvertor.h"

#include <QWidget>
#include <QComboBox>
#include <QMutex>
#include <QMap>
#include <QPoint>

#include "soundplayer.h"

namespace Ui
{
class QQMsgTip;
}

class QQMsgTip : public QWidget
{
    Q_OBJECT
public:
    QQMsgTip(QWidget *parent = 0);

signals:
    void addItemDone();
    void activatedChatDlg(QQMsg::MsgType type, QString talk_to, QString gcode);
    void activateFriendRequestDlg(QQMsg* msg);
    void activateGroupRequestDlg(QQMsg *msg);
    void bibibi(SoundPlayer::SoundType type);

public slots:
    void slotActivated(int index);

public:
    void pushMsg(QQMsg *msg);
    void addItem(QQMsg *msg);
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
    Ui::QQMsgTip *ui;
    QMutex lock;
    NameConvertor *convertor_;

    QPoint distance_pos_;
};


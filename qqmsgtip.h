#pragma once

#include "qqmsg.h"
#include "nameconvertor.h"

#include <QWidget>
#include <QComboBox>
#include <QMutex>
#include <QMap>
#include <QPoint>

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
    void bibibi();

public slots:
    void openChatDlg(int index);

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
    void beginBibibi();

private:
    Ui::QQMsgTip *ui;
    QMutex lock;
    NameConvertor *convertor_;

    QPoint distance_pos_;
};


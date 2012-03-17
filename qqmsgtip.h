#pragma once

#include "qqmsg.h"
#include "nameconvertor.h"

#include <QWidget>
#include <QComboBox>
#include <QMutex>
#include <QMap>

class QQMsgTip : public QWidget
{
    Q_OBJECT
public:
    QQMsgTip(QWidget *parent = 0);

signals:
    void addItemDone();
    void activatedChatDlg(QQMsg::MsgType type, QString talk_to);

public slots:
    void openChatDlg(int index);

public:
    void pushMsg(QQMsg *msg);
    void addItem(QQMsg *msg);
    void setConvertor(NameConvertor *convertor)
    { convertor_ = convertor; }

private:
    QComboBox box_;
    QMutex lock;
    NameConvertor *convertor_;
};


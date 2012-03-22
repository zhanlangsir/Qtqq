#pragma once

#include <QString>

class QQMsg;

class QQMsgListener
{
public:
    virtual ~QQMsgListener() {}

public:
    virtual void showOldMsg(QVector<QQMsg*> msg) = 0;
    virtual void showMsg(const QQMsg *msg) = 0;
    virtual QString id() const = 0;
};


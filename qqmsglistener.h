#pragma once

#include <QString>

#include "qqmsg.h"

class QQMsgListener
{
public:
    virtual ~QQMsgListener() {}

public:
    virtual void showOldMsg(QVector<ShareQQMsgPtr> msg) = 0;
    virtual void showMsg(const ShareQQMsgPtr msg) = 0;
    virtual QString id() const = 0;
};


#ifndef QQCHATLOG_H
#define QQCHATLOG_H

#include <QVector>

#include "qqmsg.h"

class QQChatLog
{
public:
    QQChatLog();

public:
    virtual QVector<ShareQQMsgPtr> getLog(int page) = 0;
    virtual int totalPage() const = 0;
    virtual int currPage() const = 0;
};

#endif // QQCHATLOG_H

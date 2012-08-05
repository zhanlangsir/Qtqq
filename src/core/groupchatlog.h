#ifndef QTQQ_GROUPCHATLOG_H
#define QTQQ_GROUPCHATLOG_H

#include <QString>

#include "qqmsg.h"
#include "qqchatlog.h"

class GroupChatLog : public QQChatLog
{
public:
    GroupChatLog(QString gid);

public:
    QVector<ShareQQMsgPtr> getLog(int page);
    int totalPage() const;
    int currPage() const;

private:
    void init();
    void parse(QByteArray &arr, QVector<ShareQQMsgPtr> &chat_logs);

private:
    QString gid_;
    int num_per_page_ ;
    int end_seq_;
    int curr_page_;
    int page_count_;

	static const int log2loacl[105];
};

#endif // QTQQ_GROUPCHATLOG_H

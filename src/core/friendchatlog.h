#ifndef FRIENDCHATLOG_H
#define FRIENDCHATLOG_H

#include "qqchatlog.h"

class FriendChatLog : public QQChatLog
{
public:
    FriendChatLog(QString to_id);

public:
    void init();
    QVector<ShareQQMsgPtr> getLog(int page);
    int totalPage() const;
    int currPage() const;

private:
    void parse(QByteArray &arr, QVector<ShareQQMsgPtr> &chat_logs);
    void decodeString(const QByteArray &arr, QByteArray &decoded);
    bool decodeUnicodeCodePoint( const QByteArray &arr, int &loc, unsigned int &unicode );
    bool decodeUnicodeEscapeSequence( const QByteArray &arr, int &loc, unsigned int &unicode );
    QByteArray codePoint2Utf8(unsigned int cp);

private:
    QString to_id_;
    int page_count_;
    int curr_page_;
};

#endif // FRIENDCHATLOG_H

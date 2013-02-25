#ifndef QTQQ_CORE_QQCHATITEM_H
#define QTQQ_CORE_QQCHATITEM_H

#include <QString>

class QQChatItem
{
public:
    enum ChatItemType {kQQFace, kFriendOffpic, kFriendCface, kGroupChatImg, kWord};

    QQChatItem(ChatItemType type, QString content);
    QQChatItem();

    ChatItemType type() const;
    void set_type(ChatItemType type);

    QString content() const;
    void set_content(QString content);
    void appendContent(const QString &content)
    {
        content_.append(content);
    }

    QString file_id() const;
    void set_file_id(QString file_id);

    QString server_ip() const;
    void set_server_ip(QString server_ip);

    QString server_port() const;
    void set_server_port(QString server_port);

    bool operator==(const QQChatItem &other);

private:
    ChatItemType type_;
    QString content_ ;
    QString file_id_;
    QString server_ip_;
    QString server_port_;
};

#endif // QTQQ_CORE_QQCHATITEM_H

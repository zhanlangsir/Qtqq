#ifndef QTQQ_CORE_QQCHATITEM_H
#define QTQQ_CORE_QQCHATITEM_H

#include <QString>

class QQChatItem
{
public:
    enum ChatItemType {kQQFace, kFriendOffpic, kFriendCface, kGroupChatImg, kWord};
    QQChatItem(ChatItemType type, QString content) : type_(type), content_(content) {}
    QQChatItem() {}

    ChatItemType type() const
    { return type_; }
    void set_type(ChatItemType type)
    { type_ = type; }

    QString content() const
    { return content_; }
    void set_content(QString content)
    {
        if (content_ != content)
            content_ = content;
    }

    QString file_id() const
    { return file_id_; }
    void set_file_id(QString file_id)
    {
        if (file_id_ != file_id)
            file_id_ = file_id;
    }

    QString server_ip() const
    { return server_ip_; }
    void set_server_ip(QString server_ip)
    {
        if (server_ip_ != server_ip)
            server_ip_ = server_ip;
    }

    QString server_port() const
    { return server_port_; }
    void set_server_port(QString server_port)
    {
        if (server_port_ != server_port)
            server_port_ = server_port;
    }

private:
    ChatItemType type_;
    QString content_ ;
    QString file_id_;
    QString server_ip_;
    QString server_port_;
};

#endif // QTQQ_CORE_QQCHATITEM_H

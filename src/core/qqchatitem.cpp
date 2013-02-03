#include "qqchatitem.h"

QQChatItem::QQChatItem(QQChatItem::ChatItemType type, QString content) : 
    type_(type), content_(content) 
{
}

QQChatItem::QQChatItem() 
{
}

QQChatItem::ChatItemType QQChatItem::type() const
{ return type_; }
void QQChatItem::set_type(QQChatItem::ChatItemType type)
{ type_ = type; }

QString QQChatItem::content() const
{ return content_; }
void QQChatItem::set_content(QString content)
{
	if (content_ != content)
		content_ = content;
}

QString QQChatItem::file_id() const
{ return file_id_; }
void QQChatItem::set_file_id(QString file_id)
{
	if (file_id_ != file_id)
		file_id_ = file_id;
}

QString QQChatItem::server_ip() const
{ return server_ip_; }
void QQChatItem::set_server_ip(QString server_ip)
{
	if (server_ip_ != server_ip)
		server_ip_ = server_ip;
}

QString QQChatItem::server_port() const
{ return server_port_; }
void QQChatItem::set_server_port(QString server_port)
{
	if (server_port_ != server_port)
		server_port_ = server_port;
}

bool QQChatItem::operator==(const QQChatItem &other)
{
	if ( this->type_ == other.type_ &&
			this->content_ == other.content_ )
		return true;

	return false;
}

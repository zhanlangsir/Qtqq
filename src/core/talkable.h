#ifndef TALKABLE_H
#define TALKABLE_H

#include <QObject>
#include <QVector>
#include <QPixmap>
#include <QVariant>

#include "utils/contact_status.h"

class Category;

enum TalkableDataRole 
{
	TDR_Id = 32,
	TDR_Gcode,
	TDR_Name,
	TDR_Category,
   	TDR_Markname,
	TDR_Icon,
	TDR_Status,
	TDR_ClientType,

	//Category	
	TDR_CategoryIndex,
	TDR_CategoryOnlineCount,
};

class Talkable : public QObject
{
	Q_OBJECT
signals:
	void sigDataChanged(QVariant data, TalkableDataRole role);

public:
	enum TalkableType { kContact, kGroup, kCategory };

	Talkable(QString id, QString name, TalkableType type) :
		id_(id),
		name_(name),
		type_(type)
	{
	}

	QString id() const
	{
		return id_;
	}

	void setName(QString name)
	{
		QString before = name_;
		if ( name_ != name )
		{
			name_ = name;
			emit sigDataChanged(name, TDR_Name);	
		}
	}
	QString name() const
	{
		return name_;
	}

	TalkableType type() const
	{
		return type_;
	}

	void setIconPath(QString icon_path) 
	{
		if ( icon_path_ != icon_path )
		{
			icon_path_ = icon_path;
			emit sigDataChanged(icon_path, TDR_Icon);
		}
	}
	void setIcon(QByteArray icon_data)
	{
		QPixmap before = pix_;

		QPixmap pix;	
		pix.loadFromData(icon_data);
		pix_ = pix;

		emit sigDataChanged(pix, TDR_Icon);
	}
	QPixmap icon() const
	{
		return pix_;
	}

	QString iconPath() const
	{ return icon_path_; }

	virtual QString gcode() const
	{ return QString(); }
	virtual ContactStatus status() const
	{ return CS_Online; }

private:
	QString id_;
	QString name_;
	QString icon_path_;
	QPixmap pix_;
	TalkableType type_;
};


class Contact : public Talkable
{
public:
	Contact(QString id, QString name) :
		Talkable(id, name, Talkable::kContact)
	{
	}

	void setCategory(Category *cat)
	{
		cat_ = cat;
	}
	Category* category() const
	{ return cat_; }

	void setMarkname(QString markname)
	{
		if ( markname_ != markname )
		{
			markname_ = markname;
			emit sigDataChanged(markname, TDR_Markname);
		}
	}
	QString markname() const
	{
		return markname_.isEmpty() ? name() : markname_;
	}

	void setStatus(ContactStatus status)
	{
		if ( status_ != status )
		{
			status_ = status;
			emit sigDataChanged(QVariant::fromValue<ContactStatus>(status), TDR_Status);
		}
	}
	virtual ContactStatus status() const
	{
		return status_;
	}

	void setClientType(ContactClientType type)
	{
		if ( client_type_ != type )
		{
			client_type_ = type;
			emit sigDataChanged(type, TDR_ClientType);
		}
	}
	ContactClientType clientType() const
	{
		return client_type_;
	}

private:
	QString markname_;

	Category *cat_;

	ContactStatus status_;
	ContactClientType client_type_;
};

class Group : public Talkable
{
public:
	Group(QString id, QString gcode, QString name) :
		Talkable(id, name, Talkable::kGroup),
		gcode_(gcode)
	{
	}

	virtual QString gcode() const
	{
		return gcode_;
	}

private:
	QString gcode_;
};

class Category 
{
public:
	Category(int index, QString name) : 
		index_(index),
		n_online_(0),
		name_(name)
	{
	}

	void setIndex(int idx)
	{
		index_ = idx;
	}
	int index() const
	{
		return index_;
	}

	int onlineCount() const
	{
		return n_online_;
	}
	void setOnlineCount(int n_online)
	{
		n_online_ = n_online;
	}
	
	void setName(QString name)
	{
		name_ = name;
	}
	QString name() const
	{
		return name_;
	}

	QVector<Contact*> contacts;
private:
	int index_;
	int n_online_;
	QString name_;
};

#endif //TALKABLE_H

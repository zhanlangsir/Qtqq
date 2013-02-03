#ifndef TALKABLE_H
#define TALKABLE_H

#include <QObject>
#include <QVector>
#include <QPixmap>
#include <QVariant>
#include <QFile>
#include <QMap>

#include "utils/contact_status.h"
#include "roster/group_presister.h"
#include "qqglobal.h"

enum TalkableDataRole 
{
    //Generate
	TDR_Id = 32,
	TDR_Name,
   	TDR_Markname,
	TDR_Avatar,

    //Contact
	TDR_Category,
	TDR_Status,
	TDR_ClientType,

    //Group
	TDR_Gcode,
    TDR_Announcement,

	//Category	
	TDR_CategoryIndex,
	TDR_CategoryOnlineCount,
};


class Talkable : public QObject
{
	Q_OBJECT
signals:
	void dataChanged(QVariant data, TalkableDataRole role);

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
			emit dataChanged(name, TDR_Name);	
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

	void setAvatarPath(QString avatar_path) 
	{
        avatar_path_ = avatar_path;

        QFile file(avatar_path);
        file.open(QIODevice::ReadOnly);
        QByteArray data = file.readAll();

        setAvatar(data);
        file.close();
	}

	void setAvatar(QByteArray data)
	{
		QPixmap pix;	
		pix.loadFromData(data);
		pix_ = pix;


        avatar_path_ = QQGlobal::tempDir() + "/avatar/" + id_ + ".jpg"; 
        pix.save(avatar_path_);

		emit dataChanged(pix, TDR_Avatar);
	}

	QPixmap avatar() const
	{
		return pix_;
	}

	QString avatarPath() const
	{ return avatar_path_; }

	virtual QString gcode() const
	{ return QString(); }
	virtual ContactStatus status() const
	{ return CS_Online; }

private:
	QString id_;
	QString name_;
	QString avatar_path_;
	QPixmap pix_;
	TalkableType type_;
};

class Category;
class Group;

class Contact : public Talkable
{
public:
	Contact(QString id, QString name) :
		Talkable(id, name, Talkable::kContact),
        cat_(NULL),
        group_(NULL)
	{
	}

	void setCategory(Category *cat)
	{
		cat_ = cat;
	}
	Category* category() const
	{ return cat_; }

    Group *group() const
    { 
        return group_; 
    }
    void setGroup(Group *group)
    {
        group_ = group;
    }

	void setMarkname(QString markname)
	{
		if ( markname_ != markname )
		{
			markname_ = markname;
			emit dataChanged(markname, TDR_Markname);
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
			emit dataChanged(QVariant::fromValue<ContactStatus>(status), TDR_Status);
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
			emit dataChanged(type, TDR_ClientType);
		}
	}
	ContactClientType clientType() const
	{
		return client_type_;
	}
    
private:
	QString markname_;
	ContactStatus status_;
	ContactClientType client_type_;

	Category *cat_;
    Group *group_;
};

class Group : public Talkable
{
    Q_OBJECT
signals:
    void memberDataChanged(Contact *member, TalkableDataRole role);
    void memberAdded(Contact *contact);
    void memberRemoved(Contact *contact);
    
public:
	Group(QString id, QString gcode, QString name) :
		Talkable(id, name, Talkable::kGroup),
		gcode_(gcode),
        n_member_(0)
	{
	}

    ~Group()
    {
        clearMembers();
    }

    void notifyMemberDataChanged(Contact *member, TalkableDataRole role)
    {
        emit memberDataChanged(member, role);
    }

	virtual QString gcode() const
	{
		return gcode_;
	}

    void setSig(QString sig)
    { sig_ = sig; }
    QString sig() const
    { return sig_; }

    void setAnnouncement(QString &announcement)
    {
        if ( announcement_ !=  announcement )
        {
            announcement_ = announcement;
            emit dataChanged(announcement, TDR_Announcement);
        }
    }
    QString announcement() const
    { return announcement_; }

    int memberCount()
    { 
        return n_member_;
    }

    QList<Contact *> members() 
    {
        if ( members_.count() == 0 && n_member_ != 0 )
        {
            depresist();
        }
        return members_.values();
    }

    Contact *member(QString id) const
    {
        return members_.value(id, NULL);
    }

    void clearMembers()
    {
        foreach ( Contact *contact, members_.values() )
        {
            delete contact;
            contact = NULL;
        }
        members_.clear();
    }

    void removeMember(QString id)
    {
        if ( members_.contains(id) )
        {
            Contact *contact =members_.take(id);
            emit memberRemoved(contact); 
        }
    }

    void addMember(Contact *contact)
    {
        if ( !members_.contains(contact->id()) )
        {
            members_.insert(contact->id(), contact);
            ++n_member_;
            emit memberAdded(contact);
        }
    }

private:
    void depresist()
    {
        GroupPresister::instance()->getGroupMember(this);
    }

private:
	QString gcode_;
    QString sig_;
    QString announcement_;

    int n_member_;

    QMap<QString, Contact *> members_;

    friend class GroupPresister;
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

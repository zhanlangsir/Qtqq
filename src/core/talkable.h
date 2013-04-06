#ifndef TALKABLE_H
#define TALKABLE_H

#include <QObject>
#include <QVector>
#include <QPixmap>
#include <QVariant>
#include <QFile>
#include <QMap>
#include <QDir>

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
    TDR_Group,

    //Group
	TDR_Gcode,
    TDR_Announcement,

	//Category	
	TDR_CategoryIndex,
	TDR_CategoryOnlineCount,
};

class Talkable: public QObject
{
	Q_OBJECT
signals:
	void dataChanged(QVariant data, TalkableDataRole role);

public:
    //kStranger是指你删除了他好友，但是他还保留你为好友的情况
    //kSessStranger是指双方都不是好友的情况,通常为群成员
	enum TalkableType { kStranger, kSessStranger, kContact, kGroup, kCategory };

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
    virtual QString markname() const
    {
        return name_;
    }

    void setType(TalkableType type)
    {
        type_ = type;
    }

	TalkableType type() const
	{
		return type_;
	}

	void setAvatarPath(QString avatar_path) 
	{
        avatar_path_ = avatar_path;
        emit dataChanged(avatar_path_, TDR_Avatar);
	}

	void setAvatar(QByteArray data)
	{
        QString avatar_dir = QQGlobal::tempDir() + "/avatar/";

        QDir avatar_qdir(avatar_dir);
        if ( !avatar_qdir.exists() )
            avatar_qdir.mkdir(avatar_dir);

        avatar_path_ = avatar_dir + id_ + ".jpg";
        QFile file(avatar_path_);
        file.open(QIODevice::WriteOnly);
        file.write(data);
        file.close();

        emit dataChanged(avatar_path_, TDR_Avatar);
	}

	QPixmap avatar() const
	{
        if ( avatar_path_.isEmpty() )
            return  QPixmap();

        QByteArray data;
        QFile file(avatar_path_);
        file.open(QIODevice::ReadOnly);
        data = file.readAll();
        file.close();

        QPixmap pix;
        pix.loadFromData(data);
        return pix;
	}

	QString avatarPath() const
	{ return avatar_path_; }

	virtual QString gcode() const
	{ return QString(); }
	virtual ContactStatus status() const
	{ return CS_Offline; }

protected:
	QString id_;
	QString name_;
	QString avatar_path_;
	TalkableType type_;
};

class Category;
class Group;

class Contact : public Talkable
{
public:
	Contact(QString id, QString name, TalkableType type = Talkable::kContact) :
		Talkable(id, name, type),
        status_(CS_Offline),
        cat_(NULL)
	{
	}

	void setCategory(Category *cat)
	{
		cat_ = cat;
	}
	Category* category() const
	{ return cat_; }

    const QVector<Group *> &groups() const
    { 
        return groups_; 
    }
    void addGroup(Group *group)
    {
        groups_.append(group);
    }

    Contact *clone()
    {
        Contact *new_cont = new Contact(id_, name_, type_);
        new_cont->avatar_path_ = avatar_path_;
        new_cont->markname_ = markname_;
        new_cont->status_ = status_;
        new_cont->client_type_ = client_type_;
        if ( !groups_.isEmpty() )
            new_cont->groups_ = groups_;
        return new_cont;
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
    QVector<Group *> groups_;
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

    QVector<Contact *> members() 
    {
        if ( members_.count() == 0 && n_member_ != 0 )
        {
            depresist();
        }
        return members_.values().toVector();
    }

    Contact *member(QString id) const
    {
        if ( members_.count() == 0 && n_member_ != 0 )
        {
            const_cast<Group *>(this)->depresist();
        }
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

            delete contact;
            contact = NULL;
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

    void addMember(QString id)
    {
        
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

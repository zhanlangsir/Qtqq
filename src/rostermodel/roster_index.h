#ifndef ROSTER_INDEX_H
#define ROSTER_INDEX_H

#include <QModelIndex>
#include <QVariant>
#include <QMap>
#include <QList>
#include <QDebug>

#include "core/talkable.h"

enum RosterIndexType
{
	RIT_Root,
	RIT_Category,
	RIT_Contact,
	RIT_Group,
    RIT_Stranger,
	RIT_Sess
};

class RosterIndex : QObject
{
	Q_OBJECT
public:
	RosterIndex(RosterIndexType type) :
		type_(type),
		parent_(NULL)
	{
	}

	int row() const
	{
		return parent_ ? parent_->childRow((RosterIndex *const)this) : -1;
	}

	int childRow(RosterIndex *child) const
	{
		return childs_.indexOf(child);
	}

	RosterIndex *child(int row) const
	{
		return childs_.value(row, NULL);
	}

	int childCount() const
	{
		return childs_.count();
	}

	void appendChild(RosterIndex *child)
	{
		if ( child && !childs_.contains(child) )
		{
			childs_.append(child);
			child->setParent(this);
		}
	}

    virtual QString gcode() const
    { return QString(); }
    virtual QString id() const
    {
        return QString();
    }

	RosterIndexType type() const
	{
		return type_;
	}

	virtual QVariant data(int role) const
	{
		return QVariant();
	}

    void setParent(RosterIndex *parent)
    {
        parent_ = parent;
    }

    RosterIndex *parent() const
    {
        return parent_;
    }

    virtual ContactStatus status() const
    {
        return CS_Offline;
    }

    QList<RosterIndex *>& childs() 
    {
        return childs_;
    }

private:
    RosterIndexType type_;

    RosterIndex *parent_;
    QList<RosterIndex *> childs_;
};

class TalkableIndex : public RosterIndex
{
public:
    TalkableIndex(Talkable *talkable, RosterIndexType type) :
        RosterIndex(type),
        talkable_(talkable)
    {
    }

	QVariant data(int role) const
	{
		switch ( role )
		{
		case Qt::DisplayRole:
			{
				return talkable_->markname(); 
			}
			break;
		case Qt::DecorationRole:
			if ( !talkable_->avatar().isNull() )
				return talkable_->avatar();

			return QVariant();
			break;
		default:
            return QVariant();
			break;
		}	

		return QVariant();
	}

    virtual QString gcode() const
    { return talkable_->gcode(); }

    virtual QString id() const
    {
        return talkable_->id();
    }

    virtual ContactStatus status() const
    {
        return talkable_->status();
    }

private:
    Talkable *talkable_;
};

class CategoryIndex : public RosterIndex
{
public:
    CategoryIndex(Category *cat, RosterIndexType type = RIT_Category) :
        RosterIndex(type),
        cat_(cat)
    {
    }

	virtual QVariant data(int role) const
	{
		switch ( role )
		{
		case Qt::DisplayRole:
			{
				QString name = cat_->name(); 
                name = name + "\t( " + 
                    QString::number(cat_->onlineCount()) + " / " + 
                    QString::number(childCount()) + " )";

				return name;
			}
			break;
		case Qt::DecorationRole:
			return QVariant();
			break;
		default:
            return QVariant();
			break;
		}	

		return QVariant();
	}

    int index() const
    { return cat_->index(); }

private:
    Category *cat_;
};

#endif //ROSTER_INDEX_H

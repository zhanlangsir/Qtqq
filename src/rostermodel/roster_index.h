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
signals:
	void sigDataChanged(QString id, QVariant data, TalkableDataRole role);
	void sigCategoryRenamed(int index, QString after_name);

public slots:
	void slotDataChanged(QString id, QVariant data, TalkableDataRole role)
	{	
		qDebug() << "data changed" << endl;
		setData(role, data);
	}


public:
	RosterIndex(RosterIndexType type) :
		type_(type),
		parent_(NULL),
        pix("/home/zhanlang/proj/qtqq/data/res/webqq.ico")
	{
	}
	~RosterIndex()
	{
		QMap<int, QVariant>::iterator itor = datas_.begin();
		while ( itor != datas_.end() )
		{
			itor->clear();
			++itor;
		}
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
			//emit childAboutToBeInserted(child);
			childs_.append(child);
			child->setParent(this);
			//emit childInserted(child);
		}
	}

	RosterIndexType type() const
	{
		return type_;
	}

	QVariant data(int role) const
	{
		switch ( role )
		{
		case Qt::DisplayRole:
			{
				QString name; 
				if ( datas_.contains(TDR_Markname) )
					name = datas_[TDR_Markname].toString();
				else
					name = datas_[TDR_Name].toString();

				if ( type() == RIT_Category )
					name = name + "\t( " + 
						QString::number(datas_[TDR_CategoryOnlineCount].toInt()) + " / " + 
						QString::number(childCount()) + " )";

				return name;
			}
			break;
		case Qt::DecorationRole:
			if ( datas_.contains(TDR_Avatar) )
			{
				return datas_[TDR_Avatar];
			}
			return QVariant();
			break;
		default:
			if ( datas_.contains(role) )
				return datas_[role];
			else
				return QVariant();
			break;
		}	

		return QVariant();
	}

	void setData(TalkableDataRole role, QVariant data)
	{
		if ( datas_[role] != data )
		{
			datas_[role] = data;
			//emit sigDataChanged(datas_[TDR_Id].toString(), data, role);
		}
    }

    void setParent(RosterIndex *parent)
    {
        parent_ = parent;
    }

    RosterIndex *parent() const
    {
        return parent_;
    }

    QList<RosterIndex *>& childs() 
    {
        return childs_;
    }

private:
    RosterIndexType type_;
    QMap<int, QVariant> datas_;

    QPixmap pix;
    RosterIndex *parent_;
    QList<RosterIndex *> childs_;
};

#endif //ROSTER_INDEX_H

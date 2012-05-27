#pragma once

#include "types.h"
#include <QString>
#include <QVector>

class QQItem
{
public:
    enum ItemType {kCategory, kFriend, kGroup};
    QQItem(ItemType type, ItemInfo *info, QQItem *parent = 0) :
        type_(type),
        info_(info),
        parent_(parent)
    {
    }
    QQItem(QQItem *parent = 0) : info_(NULL), parent_(parent) {}
    virtual ~QQItem()
    {
        if (info_)
        {
            delete info_;
            info_ = NULL;
        }

        QQItem *item = NULL;
        foreach(item , children_)
        {
            if (item)
            {
                delete item;
                item = NULL;
            }
        }
    }

public:
    QString avatarPath() const
    { return info_->avatarPath(); }
    void set_avatarPath(QString avatar_path)
    { info_->set_avatarPath(avatar_path); }

    ItemType type() const
    { return type_; }
    void set_type(ItemType type)
    { type_ = type; }

    QQItem *parent() const
    { return parent_; }
    void set_parent(QQItem *parent)
    { parent_ = parent; }

    QString name() const
    { return info_->name(); }


    QString markName() const
    { return info_->markName(); }

    QString id() const
    { return info_->id(); }

    QString mood() const
    { return info_->mood(); }

    QString gCode() const
    { return info_->gCode(); }

    ItemInfo *itemInfo() const
    {   return info_; }
    void set_itemInfo(ItemInfo *info)
    { info_ = info; }

    FriendStatus status() const
    { return info_->status(); }
    void set_status(FriendStatus status)
    { info_->set_status(status);}

    ClientType clientType() const
    { return info_->clientType(); }
    void set_clientType(ClientType type)
    { info_->set_clientType(type); }

    void append(QQItem *item)
    { children_.append(item); }
    void insert(int i, QQItem *item)
    { children_.insert(i, item); }
    int count() const
    { return children_.count(); }
    int indexOf(QQItem *item) const
    { return children_.indexOf(item); }
    QQItem* value(int i) const
    { return children_.value(i); }

public:
    QVector<QQItem*>children_;

private:
    ItemType type_;
    ItemInfo *info_;
    QQItem *parent_;
};


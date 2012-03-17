#pragma once

#include "types.h"
#include <QString>
#include <QVector>

class QQItem
{
public:
    enum ItemType {kCategory, kFriend, kGroup};
    QQItem(ItemType type, ItemInfo *item, QQItem *parent = 0) :
        type_(type),
        item_(item),
        parent_(parent)
    {
    }
    QQItem(QQItem *parent = 0) : parent_(parent) {}

public:
    ItemType type() const
    { return type_; }
    void set_type(ItemType type)
    { type_ = type; }

    QQItem *parent() const
    { return parent_; }
    void set_parent(QQItem *parent)
    { parent_ = parent; }

    QString name() const
    { return item_->name(); }

    QString id() const
    { return item_->id(); }

    QString mood() const
    { return item_->mood(); }

    const ItemInfo *itemInfo() const
    {   return item_; }
    void set_itemInfo(ItemInfo *item)
    { item_ = item; }

    FriendStatus state() const
    { return item_->state(); }
    void set_state(FriendStatus state)
    { item_->set_state(state);}

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
    ItemInfo *item_;
    QQItem *parent_;
};


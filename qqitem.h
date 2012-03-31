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
    QQItem(QQItem *parent = 0) : parent_(parent) {}

public:
    QQItem* shallowCopy()
    {
        QQItem *item = new QQItem(type(), itemInfo(), parent());
        return item;
    }

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

    QString id() const
    { return info_->id(); }

    QString mood() const
    { return info_->mood(); }

    ItemInfo *itemInfo() const
    {   return info_; }
    void set_itemInfo(ItemInfo *info)
    { info_ = info; }

    FriendStatus state() const
    { return info_->state(); }
    void set_state(FriendStatus state)
    { info_->set_state(state);}

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


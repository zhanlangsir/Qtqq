#ifndef QQITEM_H
#define QQITEM_H

#include <QString>
#include <QVector>

#include "utils/contact_status.h"

class QQItem
{
public:
    enum ItemType {kCategory, kFriend, kGroup};

    QQItem(ItemType type, QString name, QString id, QQItem *parent = 0) :
        type_(type),
        parent_(parent),
        id_(id),
        name_(name),
        online_count_(0)
    {
    }
    QQItem(QQItem *parent = 0) : parent_(parent), online_count_(0) {}

    ~QQItem()
    {
        foreach(QQItem *item , children_)
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
    {
        return avatar_path_;
    }
    void set_avatarPath(QString avatar_path)
    { avatar_path_ = avatar_path; }

    ItemType type() const
    { return type_; }
    void set_type(ItemType type)
    { type_ = type; }

    QQItem *parent() const
    { return parent_; }
    void set_parent(QQItem *parent)
    { parent_ = parent; }

    QString name() const
    { return name_; }
    void set_name(QString name)
    { name_ = name; }

    void setOnlineCount(int online_count)
    { online_count_ = online_count; }
    int onlineCount() const
    { return online_count_; }

    QString gCode() const
    { return gcode_; }
    void set_gCode(QString gcode)
    { gcode_ = gcode; }

    ContactStatus status() const
    { return status_; }
    void set_status(ContactStatus status)
    { status_ = status; }

    ContactClientType clientType() const
    { return client_type_; }
    void set_clientType(ContactClientType type)
    { client_type_ = type;  }

    QString id() const
    { return id_; }
    void set_id(QString id)
    { id_ = id; }

    QString mood() const
    { return mood_; }
    void set_mood(QString mood)
    { mood_ = mood; }

    void set_markName(QString mark_name)
    { mark_name_ = mark_name; }
    QString markName() const
    {
        if (mark_name_.isEmpty())
            return name_;

        return mark_name_;
    }

public:
    QVector<QQItem*>children_;

private:
    ItemType type_;
    QQItem *parent_;

    QString id_;
    QString name_;    
    QString mood_;
    QString avatar_path_;
    int online_count_;

    QString mark_name_;
    ContactStatus status_;
    ContactClientType client_type_;

    QString gcode_;
};

#endif //QQITEM_H

#pragma once

#include <QToolButton>
#include <QString>

enum ItemType {Friend, Group};

class FriendItem : public QToolButton
{
    Q_OBJECT

public:
    FriendItem(QString name, QString uin, QString mood)
    {
        name_ = name;
        uin_ = uin;
        mood_ = mood;
    }

public:
    ItemType type() { return type_; }
    void set_type();
    QString uin() { return uin_; }

private slots:
    


private:
    ItemType type_;       
    QString name_;
    QString mood_;
    QString uin_;
};

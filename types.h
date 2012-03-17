#pragma once

#include <QString>
#include <QVector>
#include <QMetaType>

enum FriendStatus {kOnline, kLeave};

struct FileInfo
{
    int size_;
    QString name_;
    QString network_path_;
};

struct CaptchaInfo
{
    QString vfwebqq_;
    QString psessionid_;
    QString skey_;
    QString cookie_;
};

class ItemInfo
{
public:
    QString name() const
    { return name_; }
    void set_name(QString name)
    { name_ = name; }

    QString id() const
    { return id_; }
    void set_id(QString id)
    { id_ = id; }

    QString mood() const
    { return mood_; }
    void set_mood(QString mood)
    { mood_ = mood; }

    virtual FriendStatus state() const
    { return kOnline; }
    virtual void set_state(FriendStatus state)
    {}

private:
    QString name_;
    QString id_;
    QString mood_;
};

class FriendInfo : public ItemInfo
{
public:
    FriendInfo() {}

public:
    FriendStatus state() const
    { return state_; }
    void set_state(FriendStatus state)
    { state_ = state; }

private:
    QString mark_name_;
    FriendStatus state_;
};

class GroupInfo : public ItemInfo
{
public:
    QString code() const
    { return code_; }
    void set_code(QString code)
    { code_ = code; }

private:
    QString code_;
};

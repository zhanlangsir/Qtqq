#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <cassert>

#include <QShortcut>
#include <QVector>
#include <QString>
#include <QKeySequence>

#include "qxtglobalshortcut.h"

#define KEY_ATTR "key"

class Shortcut;

struct SCGroup
{
    QString id;
    QString descript;
    int order;

    QVector<Shortcut *> shortcuts;
};

class Shortcut : public QObject
{
    Q_OBJECT
signals:
    void activated();

public:
    enum ShortcutType { Normal, Global };

    Shortcut(const QString &id, const QString &desc, const QString &key, ShortcutType type) :
        id_(id),
        descript_(desc),
        key_(key),
        type_(type),
        group_(NULL)
    {
    }

    virtual void setKey(const QString &key) = 0;

    virtual void setEnabled(bool enabled) = 0;

    void setGroup(SCGroup *group)
    {
        assert(group != NULL);
        group->shortcuts.append(this);

        group_ = group;
    }
    SCGroup *group() const
    { 
        return group_;
    }

    QString id() const
    {
        return id_;
    }

    QString descript() const
    {
        return descript_;
    }

    QString key() const
    {
        return key_;
    }

    ShortcutType type() const
    {
        return type_; 
    }

protected:
    QString id_;
    QString descript_;
    QString key_;

    SCGroup *group_;

    ShortcutType type_;
};

class NormalShortcut : public Shortcut
{
public:
    NormalShortcut(const QString &id, const QString &desc, const QString &key, QWidget *wid = 0) :
        Shortcut(id, desc, key, Shortcut::Normal),
        shortcut_(key, wid)
    {
        connect(&shortcut_, SIGNAL(activated()), this, SIGNAL(activated()));
    }

    virtual void setKey(const QString &key)
    {
        shortcut_.setKey(QKeySequence(key));
        key_ = key;
    }

    virtual void setEnabled(bool enabled)
    {
        shortcut_.setEnabled(enabled);
    }

private:
    QShortcut shortcut_;
};

class GlobalShortcut : public Shortcut
{
public:
    GlobalShortcut(const QString &id, const QString &desc, const QString &key, QObject *parent = 0) :
        Shortcut(id, desc, key, Shortcut::Global),
        shortcut_(key, parent)

    {
        connect(&shortcut_, SIGNAL(activated()), this, SIGNAL(activated()));
    }

    virtual void setKey(const QString &key)
    {
        shortcut_.setShortcut(key);
        key_ = key;
    }

    virtual void setEnabled(bool enabled)
    {
        shortcut_.setEnabled(enabled);
    }

private:
    QxtGlobalShortcut shortcut_;
};

#endif //SHORTCUT_H

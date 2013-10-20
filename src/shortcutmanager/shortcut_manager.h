#ifndef SHORTCUT_MANAGER_H
#define SHORTCUT_MANAGER_H

#include <QMap>
#include <QList>
#include <QDomDocument>
#include <QDomElement>
#include <QAction>

#include "shortcut.h"
#include "shortcuts_dlg.h"
#include "mainwindow.h"
#include "utils/menu.h"
#include "setting/setting.h"
#include "qtqq.h"

#define SHORTCUT_ELEMENT_TAG "Shortcuts"

#define SCG_GLOBAL "Global"
#define SCG_NORMAL "Normal"

class ShortcutManager : public QObject
{
    Q_OBJECT
signals:
    void shortcutAdded(const Shortcut *sc, const SCGroup *group);
    void groupAdded(const SCGroup *group);

    void shortcutRemoved(const Shortcut *sc, const SCGroup *group);

public:
	static ShortcutManager* instance()
	{
		if ( !instance_ )
			instance_ = new ShortcutManager();

		return instance_;
	}

    QString keyFromShortcutId(const QString &id, const QString &sc_group_id, const QString &default_key = "") const
    {
        QDomElement shortcuts_elem = Setting::instance()->documentElement().firstChildElement(SHORTCUT_ELEMENT_TAG);
        if ( shortcuts_elem.isNull() )
        {
            qDebug() << "No Shortcut Element in Setting! \n" << endl;
            return default_key;
        }

        QDomElement group_elem = shortcuts_elem.firstChildElement();
        if ( shortcuts_elem.isNull() )
        {
            qDebug() << "No Such Shortcut Group Element in Setting!: " << sc_group_id << endl;
            return default_key;
        }

        QDomElement sc_elem = group_elem.firstChildElement(id);
        if ( !sc_elem.isNull() )
        {
            return sc_elem.attribute(KEY_ATTR);
        }
        else
        {
            return default_key;
        }
    }

    bool createGroup(const QString &group_id, const QString &descript)
    {
        SCGroup *sc_group = new SCGroup();
        sc_group->id = group_id;
        sc_group->descript = descript;
        sc_group->order = group_order++;

        groups_.insert(sc_group->id, sc_group);

        emit groupAdded(sc_group);
        return true;
    }

    QList<SCGroup *> groups() const
    {
        return groups_.values();
    }

    const Shortcut *createWidgetShortcut(const QString &id, const QString &group_id, const QString &descript, const QKeySequence &key, QWidget *wid)
    {
        SCGroup *group = groups_.value(group_id, NULL);
        if ( !group )
            return NULL;

        Shortcut *sc = new NormalShortcut(id, descript, key.toString(), wid);
        sc->setGroup(group);
        shortcuts_.insert(sc->id(), sc);

        emit shortcutAdded(sc, group);
        return sc;
    }

    const GlobalShortcut *createGlobalShortcut(const QString &id, const QString &descript, const QKeySequence &key, QWidget *parent = 0)
    {
        SCGroup *group = groups_.value(SCG_GLOBAL, NULL);
        if ( !group )
            return NULL;

        GlobalShortcut *g_sc = new GlobalShortcut(id, descript, key.toString(), parent);
        g_sc->setGroup(group);
        shortcuts_.insert(g_sc->id(), g_sc);

        emit shortcutAdded(g_sc, group);
        return g_sc;
    }

    int stopShortcut(const QString &id)
    {
        Q_UNUSED(id)
        return 0; 
    }

    int removeShortcut(const QString &id)
    {
        Shortcut *sc = shortcuts_.value(id, NULL);
        if ( sc )
        {
            sc->setEnabled(false);
            delete sc;
            sc = NULL;
        }
        emit shortcutRemoved(sc, sc->group());

        return shortcuts_.remove(id);
    }

    void updateShortcut(const QString &id, const QKeySequence &key)
    {
        if ( shortcuts_.contains(id) )
        {
            shortcuts_[id]->setKey(key.toString());
        }
    }
    void updateShortcuts(const QVector<Shortcut *> shortcuts)
    {
        Q_UNUSED(shortcuts)
    }

    Shortcut *shortcut(const QString &id) const
    {
        return shortcuts_.value(id, NULL);
    }

private slots:
    void openShortcutDlg()
    {
        shortcut_dlg_ = new ShortcutDlg();
        shortcut_dlg_->init();
        shortcut_dlg_->show();
    }

private:
    ShortcutDlg *shortcut_dlg_;

    QMap<QString ,SCGroup *> groups_;
    QMap<QString, Shortcut *> shortcuts_;

    static int group_order;

private:
	ShortcutManager()
    {
        MainWindow *main_win = Qtqq::instance()->mainWindow();

        QAction *act_open_hotkey_dlg = new QAction(tr("Open Shortcut Dialog"), main_win->mainMenu());
        act_open_hotkey_dlg->setCheckable(false);
        connect(act_open_hotkey_dlg, SIGNAL(triggered()), this, SLOT(openShortcutDlg()));
        main_win->mainMenu()->addPluginAction(act_open_hotkey_dlg);

        createGroup(SCG_GLOBAL, tr("Global Shortcut"));
        createGroup(SCG_NORMAL, tr("Normal Shortcut"));
    }

   	ShortcutManager(const ShortcutManager&);
	ShortcutManager& operator=(const ShortcutManager&);

	static ShortcutManager* instance_;
};

#endif //SHORTCUT_MANAGER_H

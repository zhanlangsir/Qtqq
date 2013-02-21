#ifndef SHORTCUT_MANAGER_H
#define SHORTCUT_MANAGER_H

class ShortcutManager : public QObject
{
    Q_OBJECT
signals:
    void globalShortcutTrigger(const QKeySequence &key);

public:
	static ShortcutManager* instance()
	{
		if ( !instance_ )
			instance_ = new ShortcutManager();
		return instance_;
	}

    bool registerGlobalShortcut(const QKeySequence &key);

private:


private:
	ShortcutManager();
   	ShortcutManager(const ShortcutManager&);
	ShortcutManager& operator=(const ShortcutManager&);

	static ShortcutManager* instance_;
};

#endif //SHORTCUT_MANAGER_H

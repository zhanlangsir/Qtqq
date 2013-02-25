#ifndef HOTKEY_MANAGER_H
#define HOTKEY_MANAGER_H

#include <QObject>
#include <QVector>

#include "hotkeymanager/hotkeys.h"

class HotkeyDlg;
class QxtGlobalShortcut;

class HotkeyManager : public QObject
{
    Q_OBJECT
public:
    static HotkeyManager *instance()
    {
        if ( !instance_ )
            instance_ = new HotkeyManager();

        return instance_;
    }
    ~HotkeyManager();

    void reset();

    QxtGlobalShortcut *hotkey(HotkeyType type)
    {
        return hotkeys_[type];
    }

private slots:
    void onHotKeyApply();
    void openHotkeyDlg();

private:
    void initHotkeys();

private:
    QVector<QxtGlobalShortcut *> hotkeys_;

    HotkeyDlg *hotkey_dlg_;

private:
    HotkeyManager();
    HotkeyManager(const HotkeyManager &);
    HotkeyManager &operator=(const HotkeyManager &);

    static HotkeyManager *instance_;
};

#endif //HOTKEY_MANAGER_H

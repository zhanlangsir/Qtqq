#include "hotkey_manager.h"
#include "ui_hotkeydlg.h"

#include <QWidget>
#include <QKeySequence>
#include <QDebug>

#include "qxtglobalshortcut.h"

#include "utils/menu.h"
#include "mainwindow.h"
#include "qtqq.h"

HotkeyManager *HotkeyManager::instance_ = NULL;

class HotkeyDlg : public QDialog, public Ui::HotkeyDlg
{
public:
    HotkeyDlg(QWidget *parent = 0) :
        QDialog(parent)
    {
        setupUi(this);
    }
};


HotkeyManager::HotkeyManager()
{
    hotkey_dlg_ = new HotkeyDlg();
    hotkey_dlg_->msg_label->setVisible(false);

    connect(hotkey_dlg_->apply_btn, SIGNAL(clicked()), this, SLOT(onHotKeyApply()));
    connect(hotkey_dlg_->ok_btn, SIGNAL(clicked()), hotkey_dlg_, SLOT(accept()));
    connect(hotkey_dlg_->cancel_btn, SIGNAL(clicked()), hotkey_dlg_, SLOT(reject()));

    initHotkeys();

    MainWindow *main_win = Qtqq::instance()->mainWindow();

    QAction *act_open_hotkey_dlg_ = new QAction(tr("Open Hotkey Dialog"), main_win->mainMenu());
    act_open_hotkey_dlg_->setCheckable(false);
    connect(act_open_hotkey_dlg_, SIGNAL(triggered()), this, SLOT(openHotkeyDlg()));
    main_win->mainMenu()->addPluginAction(act_open_hotkey_dlg_);
}

void HotkeyManager::initHotkeys()
{
    QxtGlobalShortcut *snapshot = new QxtGlobalShortcut(QKeySequence("Ctrl+Alt+S"), this);
    hotkeys_.append(snapshot);
}

void HotkeyManager::onHotKeyApply()
{
    QString hot_key = hotkey_dlg_->key_editor->text();
    if ( hot_key.toUpper() == "Z" )
    {
        hotkey_dlg_->msg_label->setText(tr("This hot key was set!"));
        hotkey_dlg_->msg_label->setVisible(true);
        return;
    }
    if ( hot_key.isEmpty() || hot_key.length() > 1 )
    {
        hotkey_dlg_->msg_label->setText(tr("Please set the correct key"));
        hotkey_dlg_->msg_label->setVisible(true);
        return;
    }

    QxtGlobalShortcut *snapshot = hotkeys_[HK_SNAPSHOT];
    QKeySequence new_ks("Ctrl+Alt+"+hot_key);
    if ( snapshot->shortcut() == new_ks )
    {
        hotkey_dlg_->msg_label->setText(tr("The same hotkey is set!"));
        hotkey_dlg_->msg_label->setVisible(true);
        return;
    }
    else
    {
        if ( !snapshot->setShortcut(new_ks) )
        {
            hotkey_dlg_->msg_label->setText(tr("Set hotkey failed, please change another key!"));
            hotkey_dlg_->msg_label->setVisible(true);
        }
        else
        {
            hotkey_dlg_->msg_label->setVisible(false);
        }
    }
}

void HotkeyManager::openHotkeyDlg()
{
    QxtGlobalShortcut *snapshot = hotkeys_[HK_SNAPSHOT];
    QKeySequence snapshot_ks = snapshot->shortcut();

    QString text = snapshot_ks.toString();
    
    if ( !text.isEmpty() )
    {
        hotkey_dlg_->key_editor->setText(text.mid(text.length()-1));
    }

    hotkey_dlg_->show();
}

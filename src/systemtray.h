#ifndef QTQQ_SYSTEMTRAY_H
#define QTQQ_SYSTEMTRAY_H

#include <QObject>
#include <QPoint>

#include "msgtip.h"

typedef struct _GtkStatusIcon GtkStatusIcon;
typedef struct _GdkPixbuf GdkPixbuf;

class QTimer;

class TrayMenu;

class SystemTray : public QObject
{
    Q_OBJECT
signals:
    void activated();

public:
    static SystemTray *instance()
    {
        if (!system_tray_)
        {
            system_tray_ = new SystemTray();
        }
        return system_tray_;
    }
    ~SystemTray();

    void show();
    void hide();

    void flicker(bool flicker);

    /*
        现在此方法无任何作用
        可以使用libnotify实现
    */
    void showMessage(const QString &title, const QString &msg, int msecs);
    void showMsgTip(QPoint pos)
    {
        if (msg_tip_)
        {
            msg_tip_->show(pos);
        }
    }

    void setIcon(const QString &file_path);
    void setContextMenu(TrayMenu *menu);

    void showMenu();
    void emitActivated();

    void setMsgTip(MsgTip *msg_tip)
    { msg_tip_ = msg_tip; }    

public slots:
    void slotNewUncheckMsgArrived();
    void slotUncheckMsgEmpty();

private:
    SystemTray(QObject *parent = 0);

private:
    static SystemTray *system_tray_;

    GtkStatusIcon *tray_icon_;
    TrayMenu *menu_;

    MsgTip *msg_tip_;
};

#endif  //QTQQ_SYSTEMTRAY_H

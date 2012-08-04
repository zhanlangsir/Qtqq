#include "systemtray.h"

#include <QTimer>
#include <QDebug>

#include "traymenu.h"

#undef signals
#include <gtk/gtk.h>

typedef int gint;
typedef gint gboolean;

static void gtkTrayIconActived(GtkStatusIcon *status_icon,
                        gpointer user_data);
static void gtkPopupMenu(GtkStatusIcon *status_icon,
                  guint button,
                  guint activate_time,
                  gpointer user_data);

static gboolean gtkQueryToolTip(GtkStatusIcon *status_icon,
                                 gint           x,
                                 gint           y,
                                 gboolean       keyboard_mode,
                                 GtkTooltip    *tooltip,
                                 gpointer       user_data);

SystemTray::SystemTray(QObject *parent) :
    QObject(parent),
    menu_(NULL),
    msg_tip_(NULL)
{
    check_cursor_pos_.setInterval(500);
    connect(&check_cursor_pos_, SIGNAL(timeout()), this, SLOT(checkCursorPos()));
    tray_icon_ = gtk_status_icon_new_from_file(PKG_DATA_DIR"/resources/WebQQ.ico");

    g_signal_connect(G_OBJECT(tray_icon_), "activate",
                     G_CALLBACK(gtkTrayIconActived), this);
    g_signal_connect(G_OBJECT(tray_icon_), "popup-menu",
                     G_CALLBACK(gtkPopupMenu), NULL);
    g_signal_connect(G_OBJECT(tray_icon_), "query-tooltip",
                     G_CALLBACK(gtkQueryToolTip), NULL);

    gtk_status_icon_set_tooltip(tray_icon_, "qtqq");

	_mainWindow = NULL;
}

SystemTray::~SystemTray()
{
    delete menu_;
    delete tray_icon_;
}

SystemTray* SystemTray::system_tray_;


void SystemTray::showMessage(const QString &title, const QString &msg, int msecs)
{
    Q_UNUSED(title)
    Q_UNUSED(msg)
    Q_UNUSED(msecs)
}

void SystemTray::setIcon(const QString &file_path)
{
   gtk_status_icon_set_from_file(tray_icon_, file_path.toLatin1().data());
}

void SystemTray::setContextMenu(TrayMenu *menu)
{
    menu_ = menu;
}

void SystemTray::showMenu()
{
    if ( menu_ )
        menu_->popup();
}

void SystemTray::emitActivated()
{
    emit activated();
}

void SystemTray::slotNewUncheckMsgArrived()
{
    flicker(true);
}

void SystemTray::slotUncheckMsgEmpty()
{
    flicker(false);
}

void SystemTray::show()
{
    gtk_status_icon_set_visible(tray_icon_, true);
}

void SystemTray::hide()
{
    gtk_status_icon_set_visible(tray_icon_, false);
}

void SystemTray::flicker(bool flicker)
{
    gtk_status_icon_set_blinking(tray_icon_, flicker);
}


void SystemTray::checkCursorPos()
{
    QPoint pos = QCursor::pos();

    if ( qAbs(pos.x() - tray_pos_.x()) > 25 )
    {
         hideMsgTip();
         check_cursor_pos_.stop();
    }
}

static void gtkTrayIconActived(GtkStatusIcon *status_icon, gpointer user_data)
{
    Q_UNUSED(status_icon)
	SystemTray *tray = (SystemTray *)user_data;
	QWidget *mainWindow = tray->getMainWindow();

	if (mainWindow)
	{
		if (mainWindow->isMinimized() || !mainWindow->isVisible())
			mainWindow->showNormal();
		else
			mainWindow->hide();
	}

    SystemTray::instance()->showMsgTip(QCursor::pos());
}

static void gtkPopupMenu(GtkStatusIcon *status_icon, guint button, guint activate_time, gpointer user_data)
{
    Q_UNUSED(status_icon)
    Q_UNUSED(button)
    Q_UNUSED(activate_time)
    Q_UNUSED(user_data)
    SystemTray::instance()->showMenu();
}

static gboolean gtkQueryToolTip(GtkStatusIcon *status_icon,
                                 gint           x,
                                 gint           y,
                                 gboolean       keyboard_mode,
                                 GtkTooltip    *tooltip,
                                 gpointer       user_data)
{
    Q_UNUSED(status_icon)
    Q_UNUSED(x)
    Q_UNUSED(y)
    Q_UNUSED(keyboard_mode)
    Q_UNUSED(tooltip)
    Q_UNUSED(user_data)

    SystemTray *tray = SystemTray::instance();
    tray->setTrayPos(QCursor::pos());
    tray->beginCheckCursorPos();
    tray->showMsgTip(QCursor::pos());
    return false;
}


/*
 *  Copyright (C) 1997-2008 Richard J. Moore <rich@kde.org>
 *  Copyright (C) 2000 Matthias Ettrich <ettrich@troll.no>
 *  Copyright (C) 2002 Aaron J. Seigo <aseigo@kde.org>
 *  Copyright (C) 2003 Nadeem Hasan <nhasan@kde.org>
 *  Copyright (C) 2004 Bernd Brandstetter <bbrand@freenet.de>
 *  Copyright (C) 2006 Urs Wolfer <uwolfer @ kde.org>
 *  Copyright (C) 2010 Martin Gräßlin <kde@martin-graesslin.com>
 *  Copyright (C) 2010, 2011 Pau Garcia i Quiles <pgquiles@elpauer.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "ksnapshot.h"
#include "ui_ksnapshotwidget.h"

#include <QClipboard>
#include <QShortcut>
#include <QFileDialog>
#include <QDir>
#include <QMenu>
#include <QIcon>
#include <QDesktopWidget>
#include <QVarLengthArray>
#include <QCloseEvent>
#include <QBuffer>
#include <QDrag>
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>
#include <QMessageBox>
#include <QVBoxLayout>

#include "regiongrabber.h"
#include "freeregiongrabber.h"
#include "windowgrabber.h"
#include "ksnapshotpreview.h"

#include "qqglobal.h"
#include "chatwidget/chatdlg_manager.h"
#include "chatwidget/qqchatdlg.h"

#ifdef HAVE_X11_EXTENSIONS_XFIXES_H
#include <X11/extensions/Xfixes.h>
#include <QX11Info>
#endif

class KSnapshotWidget : public QWidget, public Ui::KSnapshotWidget
{
public:
    KSnapshotWidget(QWidget *parent = 0)
        : QWidget(parent)
    {
        setupUi(this);
        btnNew->setIcon(QIcon("ksnapshot"));
    }
};

KSnapshot::KSnapshot(QWidget *parent,  KSnapshotObject::CaptureMode mode )
  : QDialog(parent), KSnapshotObject(), modified(true), savedPosition(QPoint(-1, -1))
{
    // TEMPORARY Make sure "untitled" enters the string freeze for 4.6, 
    // as explained in http://lists.kde.org/?l=kde-graphics-devel&m=128942871430175&w=2
    const QString untitled = QString(tr("untitled"));
    
    setWindowTitle(""); 
    grabber = new QWidget( 0,  Qt::X11BypassWindowManagerHint );
    
    // TODO X11 (Xinerama and Twinview, actually) and Windows use different coordinates for the two monitors case
    //
    // On Windows, there are two displays. The origin (0, 0) ('o') is the top left of display 1. If display 2 is to the left, then coordinates in display 2 are negative:
    //  .-------.
    //  |       |o-----. 
    //  |   2   |      |
    //  |       |   1  |
    //  ._______.._____.
    //
    // On Xinerama and Twinview, there is only one display and two screens. The origin (0, 0) ('o') is the top left of the display:
    //  o-------.
    //  |       |.-----. 
    //  |   2   |      |
    //  |       |   1  |
    //  ._______.._____.
    //
    // Instead of moving to (-10000, -10000), we should compute how many displays are and make sure we move to somewhere out of the total coordinates. 
    //   - Windows: use GetSystemMetrics ( http://msdn.microsoft.com/en-us/library/ms724385(v=vs.85).aspx )

    // If moving to a negative position, we need to count the size of the dialog; moving to a positive position avoids having to compute the size of the dialog

    grabber->move( -10000, -10000 ); // FIXME Read above

    grabber->installEventFilter( this );

    QVBoxLayout *vbox = new QVBoxLayout( this );
    setLayout(vbox);
    mainWidget = new KSnapshotWidget();
    vbox->addWidget(mainWidget);

    connect(mainWidget->ok_btn, SIGNAL(clicked()), SLOT(onOkBtnClicked()));
    connect(mainWidget->cancel_btn, SIGNAL(clicked()), SLOT(onCancelBtnClicked()));
    connect(mainWidget->save_btn, SIGNAL(clicked()), SLOT(onSaveBtnClicked()));
    connect(mainWidget->help_btn, SIGNAL(clicked()), SLOT(onHelpBtnClicked()));
    connect(mainWidget->lblImage, SIGNAL(startDrag()), SLOT(slotDragSnapshot()));
    connect(mainWidget->btnNew, SIGNAL(clicked()), SLOT(slotGrab()));
    connect(mainWidget->comboMode, SIGNAL(activated(int)), SLOT(slotModeChanged(int)));

    if (qApp->desktop()->numScreens() < 2) {
        mainWidget->comboMode->removeItem(CurrentScreen);
    }

    mainWidget->spinDelay->setSuffix(tr(" second", " seconds"));

    grabber->show();
    grabber->grabMouse();

#ifdef HAVE_X11_EXTENSIONS_XFIXES_H
    {
        int tmp1, tmp2;
        //Check whether the XFixes extension is available
        Display *dpy = QX11Info::display();
        if (!XFixesQueryExtension( dpy, &tmp1, &tmp2 )) {
            mainWidget->cbIncludePointer->hide();
            mainWidget->lblIncludePointer->hide();
        }
    }
#elif !defined(Q_WS_WIN)
    mainWidget->cbIncludePointer->hide();
    mainWidget->lblIncludePointer->hide();
#endif
    setMode(KSnapshotObject::Region);

    qDebug() << "Mode = " << mode;
    if ( mode == KSnapshotObject::FullScreen ) {
        snapshot = QPixmap::grabWindow( QApplication::desktop()->winId() );
#ifdef HAVE_X11_EXTENSIONS_XFIXES_H
        if ( haveXFixes && includePointer() )
            grabPointerImage(0, 0);
#endif
    }
    else if ( mode == KSnapshotObject::CurrentScreen ) {
        qDebug() << "Desktop Geom = " << QApplication::desktop()->geometry();
        QDesktopWidget *desktop = QApplication::desktop();
        int screenId = desktop->screenNumber( QCursor::pos() );
        qDebug() << "Screenid = " << screenId;
        QRect geom = desktop->screenGeometry( screenId );
        qDebug() << "Geometry = " << screenId;
        snapshot = QPixmap::grabWindow( desktop->winId(),
                geom.x(), geom.y(), geom.width(), geom.height() );
    }
    else {
        setMode( mode );
        switch(mode)
        {
            case KSnapshotObject::WindowUnderCursor:
                {
                    setIncludeDecorations( true );
                    performGrab();
                    break;
                }
            case  KSnapshotObject::ChildWindow:
                {
                    slotGrab();
                    break;
                }
            case KSnapshotObject::Region:
                {
                    grabRegion();
                    break;
                }
            case KSnapshotObject::FreeRegion:
            {
                 grabFreeRegion();
                 break;
            }
            default:
                break;
        }
    }

    //When we use argument to take snapshot we mustn't hide it.
    if (mode !=  KSnapshotObject::ChildWindow) {
       grabber->releaseMouse();
       grabber->hide();
    }

    setDelay(0);
    file_path_ = QQGlobal::tempDir() + "/snapshot/snapshot.png";

    connect( &grabTimer, SIGNAL(timeout()), this, SLOT(grabTimerDone()) );
    connect( &updateTimer, SIGNAL(timeout()), this, SLOT(updatePreview()) );
    QTimer::singleShot( 0, this, SLOT(updateCaption()) );

    new QShortcut( Qt::Key_S, mainWidget->ok_btn, SLOT(animateClick()));
    new QShortcut( Qt::Key_N, mainWidget->btnNew, SLOT(animateClick()) );
    new QShortcut( Qt::Key_Space, mainWidget->btnNew, SLOT(animateClick()) );

    mainWidget->ok_btn->setFocus();
    resize(QSize(400, 500));

    move((QApplication::desktop()->width() - this->width()) /2, (QApplication::desktop()->height() - this->height()) /2);
}

KSnapshot::~KSnapshot()
{
    delete mainWidget;
}

void KSnapshot::onOkBtnClicked()
{
    QQChatDlg *current_chatdlg = ChatDlgManager::instance()->currentChatdlg();
    if ( current_chatdlg )
    {
        file_path_ = getUnexistsFilePath(file_path_);
        snapshot.save(file_path_);

        current_chatdlg->insertImage(file_path_);
    }

    deleteLater();
}

void KSnapshot::onCancelBtnClicked()
{
    deleteLater();
}

void KSnapshot::resizeEvent( QResizeEvent * )
{
    updateTimer.setSingleShot( true );
    updateTimer.start( 200 );
}

void KSnapshot::onHelpBtnClicked()
{
    QMessageBox::information(NULL, tr("Help"), 
            tr(" * 点击发送可把图片发送到当前激活的聊天窗口，否则保存到默认位置，不做其他处理\n"
                " * 保存功能只能保存png,和jpg后缀，如果不加后缀，默认为png"));
}

void KSnapshot::onSaveBtnClicked()
{
    QString save_path = QFileDialog::getSaveFileName(this, tr("select the image to save"), QString(), tr("Image Files(*.png *.jpg)"));
    QFileInfo info(save_path);
    QString save_dir = info.absolutePath();
    qDebug() << save_dir << endl;

    QString suffix;
    if ( info.suffix().isEmpty() )
        suffix = "png";
    else if ( info.suffix() != "png" || info.suffix() != "jpg" )
    {
        suffix = "png";
    }

    snapshot.save(save_dir + '/' + info.baseName() + '.' + suffix);
}

void KSnapshot::slotCopy()
{
    QClipboard *cb = QApplication::clipboard();
    cb->setPixmap( snapshot );
}

void KSnapshot::slotDragSnapshot()
{
    QDrag *drag = new QDrag(this);

    drag->setMimeData(new QMimeData);
    drag->mimeData()->setImageData(snapshot);
    drag->mimeData()->setData("application/x-kde-suggestedfilename", QFileInfo(file_path_).fileName().toUtf8());
    drag->setPixmap(preview());
    drag->start();
}

void KSnapshot::slotGrab()
{
    savedPosition = pos();
    hide();

    if (delay()) {
        //kDebug() << "starting timer with time of" << delay();
        grabTimer.start(delay());
    }
    else {
        QTimer::singleShot(0, this, SLOT(startUndelayedGrab()));
    }
}

void KSnapshot::startUndelayedGrab()
{
    if (mode() == Region) {
        grabRegion();
    }
    else if ( mode() == FreeRegion ) {
        grabFreeRegion();
    }
    else {
        grabber->show();
        grabber->grabMouse(Qt::CrossCursor);
    }
}

QString KSnapshot::getUnexistsFilePath(const QString &base)
{
    QFileInfo save_info(base);
    QString basename = save_info.baseName();
    QString suffix = save_info.suffix();
    if ( !suffix.isEmpty() )
        suffix = '.' + suffix;

    QString save_dir = QQGlobal::tempDir() +"/snapshot";
    QDir snap_qdir(save_dir);
    if ( !snap_qdir.exists() )
        snap_qdir.mkdir(save_dir);

    int i = 1;
    QString base_path = save_dir + '/' + basename;
    QString final_save_path = base_path + suffix;

    QDir save_qdir(save_dir);
    while ( save_qdir.exists(final_save_path) )
    {
        final_save_path = base_path + QString::number(i) + suffix;
        ++i;
    }

    return final_save_path;
}

void KSnapshot::slotRegionGrabbed( const QPixmap &pix )
{
    if ( !pix.isNull() )
    {
        snapshot = pix;
        updatePreview();
        modified = true;
        updateCaption();
    }

    if ( snapshot.isNull() )
    {
        mainWidget->ok_btn->setEnabled(false);
        mainWidget->save_btn->setEnabled(false);
    }
    else
    {
        mainWidget->ok_btn->setEnabled(true);
        mainWidget->save_btn->setEnabled(true);
    }

    if( mode() == KSnapshotObject::Region )
    {
        rgnGrab->deleteLater();
    }
    else if( mode() == KSnapshotObject::FreeRegion ) {
        freeRgnGrab->deleteLater();
    }

    QApplication::restoreOverrideCursor();
    show();
}

void KSnapshot::slotWindowGrabbed( const QPixmap &pix )
{
    if ( !pix.isNull() )
    {
        snapshot = pix;
        updatePreview();
        modified = true;
        updateCaption();

    }

    if ( snapshot.isNull() )
    {
        mainWidget->ok_btn->setEnabled(false);
        mainWidget->save_btn->setEnabled(false);
    }
    else
    {
        mainWidget->ok_btn->setEnabled(true);
        mainWidget->save_btn->setEnabled(true);
    }

    QApplication::restoreOverrideCursor();
    show();
}

void KSnapshot::slotScreenshotReceived( qulonglong handle )
{
#ifdef Q_WS_X11
    slotWindowGrabbed( QPixmap::fromX11Pixmap( handle ) );
#endif
}

void KSnapshot::closeEvent( QCloseEvent * e )
{
    Q_UNUSED(e)

    deleteLater();
}

bool KSnapshot::eventFilter( QObject* o, QEvent* e)
{
    if ( o == grabber && e->type() == QEvent::MouseButtonPress ) {
        QMouseEvent* me = (QMouseEvent*) e;
        if ( QWidget::mouseGrabber() != grabber )
            return false;
        if ( me->button() == Qt::LeftButton )
            performGrab();
    }
    return false;
}

void KSnapshot::updatePreview()
{
    setPreview( snapshot );
}

void KSnapshot::grabRegion()
{
   rgnGrab = new RegionGrabber();
   connect( rgnGrab, SIGNAL(regionGrabbed(QPixmap)),
                     SLOT(slotRegionGrabbed(QPixmap)) );

}

void KSnapshot::grabFreeRegion()
{
   freeRgnGrab = new FreeRegionGrabber();
   connect( freeRgnGrab, SIGNAL(freeRegionGrabbed(QPixmap)),
                     SLOT(slotRegionGrabbed(QPixmap)) );

}

void KSnapshot::grabTimerDone()
{
    if ( mode() == Region ) {
        grabRegion();
    }
    else if ( mode() == FreeRegion ) {
        grabFreeRegion();
    }
    else {
        performGrab();
    }
    //KNotification::beep(i18n("The screen has been successfully grabbed."));
}

void KSnapshot::performGrab()
{
    int x = 0;
    int y = 0;

    grabber->releaseMouse();
    grabber->hide();
    grabTimer.stop();

    title.clear();
    windowClass.clear();

    if ( mode() == ChildWindow ) {
        WindowGrabber wndGrab;
        connect( &wndGrab, SIGNAL(windowGrabbed(QPixmap)),
                           SLOT(slotWindowGrabbed(QPixmap)) );
        wndGrab.exec();
        QPoint offset = wndGrab.lastWindowPosition();
        x = offset.x();
        y = offset.y();
        qDebug() << "last window position is" << offset;
    }
    else if ( mode() == WindowUnderCursor ) {
        snapshot = WindowGrabber::grabCurrent( includeDecorations() );

        QPoint offset = WindowGrabber::lastWindowPosition();
        x = offset.x();
        y = offset.y();

        // If we're showing decorations anyway then we'll add the title and window
        // class to the output image meta data.
        if ( includeDecorations() ) {
            title = WindowGrabber::lastWindowTitle();
        }
    }
    else if ( mode() == CurrentScreen ) {
        qDebug() << "Desktop Geom2 = " << QApplication::desktop()->geometry();
        QDesktopWidget *desktop = QApplication::desktop();
        int screenId = desktop->screenNumber( QCursor::pos() );
        qDebug() << "Screenid2 = " << screenId;
        QRect geom = desktop->screenGeometry( screenId );
        qDebug() << "Geometry2 = " << geom;
        x = geom.x();
        y = geom.y();
        snapshot = QPixmap::grabWindow( desktop->winId(),
                x, y, geom.width(), geom.height() );
    }
    else {
        snapshot = QPixmap::grabWindow( QApplication::desktop()->winId() );
    }
#ifdef HAVE_X11_EXTENSIONS_XFIXES_H
    if (haveXFixes && includePointer()) {
        grabPointerImage(x, y);
    }
#endif // HAVE_X11_EXTENSIONS_XFIXES_H

    if ( snapshot.isNull() )
    {
        mainWidget->ok_btn->setEnabled(false);
        mainWidget->save_btn->setEnabled(false);
    }
    else
    {
        mainWidget->ok_btn->setEnabled(true);
        mainWidget->save_btn->setEnabled(true);
    }

    updatePreview();
    QApplication::restoreOverrideCursor();
    modified = true;
    updateCaption();
    if (savedPosition != QPoint(-1, -1)) {
        move(savedPosition);
    }
    show();
}

void KSnapshot::grabPointerImage(int offsetx, int offsety)
// Uses the X11_EXTENSIONS_XFIXES_H extension to grab the pointer image, and overlays it onto the snapshot.
{
#ifdef HAVE_X11_EXTENSIONS_XFIXES_H
    XFixesCursorImage *xcursorimg = XFixesGetCursorImage( QX11Info::display() );
    if ( !xcursorimg )
      return;

    //Annoyingly, xfixes specifies the data to be 32bit, but places it in an unsigned long *
    //which can be 64 bit.  So we need to iterate over a 64bit structure to put it in a 32bit
    //structure.
    QVarLengthArray< quint32 > pixels( xcursorimg->width * xcursorimg->height );
    for (int i = 0; i < xcursorimg->width * xcursorimg->height; ++i)
        pixels[i] = xcursorimg->pixels[i] & 0xffffffff;

    QImage qcursorimg((uchar *) pixels.data(), xcursorimg->width, xcursorimg->height,
                       QImage::Format_ARGB32_Premultiplied);

    QPainter painter(&snapshot);
    painter.drawImage(QPointF(xcursorimg->x - xcursorimg->xhot - offsetx, xcursorimg->y - xcursorimg ->yhot - offsety), qcursorimg);

    XFree(xcursorimg);
#else // HAVE_X11_EXTENSIONS_XFIXES_H
    Q_UNUSED(offsetx);
    Q_UNUSED(offsety);
    return;
#endif // HAVE_X11_EXTENSIONS_XFIXES_H
}

void KSnapshot::setTime(int newTime)
{
    setDelay(newTime);
}

int KSnapshot::timeout() const
{
    return delay();
}

void KSnapshot::setURL( const QString &url )
{
    changeUrl( url );
}

void KSnapshot::setGrabMode( int m )
{
    setMode( m );
}

int KSnapshot::grabMode() const
{
    return mode();
}

void KSnapshot::refreshCaption()
{
    updateCaption();
}

void KSnapshot::updateCaption()
{
    setWindowTitle( "Snapshot" );
    //setWindowTitle( QFileInfo(filename).fileName()+"[*]" );
    //setWindowModified(modified);
}

void KSnapshot::slotMovePointer(int x, int y)
{
    QCursor::setPos( x, y );
}

void KSnapshot::exit()
{
    reject();
}

void KSnapshot::slotModeChanged(int mode)
{
    mainWidget->cbIncludePointer->setEnabled(!(mode == Region || mode == FreeRegion));
    mainWidget->lblIncludePointer->setEnabled(!(mode == Region || mode == FreeRegion));
    mainWidget->cbIncludeDecorations->setEnabled(mode == WindowUnderCursor);
    mainWidget->lblIncludeDecorations->setEnabled(mode == WindowUnderCursor);
}

void KSnapshot::setPreview( const QPixmap &pm )
{
    mainWidget->lblImage->setToolTip( QString(tr( "Preview of the snapshot image (%1 x %2)").arg(pm.width()).arg(pm.height())) );

    mainWidget->lblImage->setPreview(pm);
    mainWidget->lblImage->adjustSize();
}

void KSnapshot::setDelay( int i )
{
    mainWidget->spinDelay->setValue(i);
}

void KSnapshot::setIncludeDecorations( bool b )
{
    mainWidget->cbIncludeDecorations->setChecked(b);
}

void KSnapshot::setIncludePointer(bool enabled)
{
    mainWidget->cbIncludePointer->setChecked(enabled);
}

bool KSnapshot::includePointer() const
{
    return mainWidget->cbIncludePointer->isChecked();
}

void KSnapshot::setMode( int mode )
{
    mainWidget->comboMode->setCurrentIndex(mode);
    slotModeChanged(mode);
}

int KSnapshot::delay() const
{
    return mainWidget->spinDelay->value();
}

bool KSnapshot::includeDecorations() const
{
    return mainWidget->cbIncludeDecorations->isChecked();
}

int KSnapshot::mode() const
{
    return mainWidget->comboMode->currentIndex();
}

QPixmap KSnapshot::preview()
{
    return *mainWidget->lblImage->pixmap();
}

int KSnapshot::previewWidth() const
{
    return mainWidget->lblImage->width();
}

int KSnapshot::previewHeight() const
{
    return mainWidget->lblImage->height();
}

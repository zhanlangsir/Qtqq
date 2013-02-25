/*
 *  Copyright (C) 1997-2002 Richard J. Moore <rich@kde.org>
 *  Copyright (C) 2000 Matthias Ettrich <ettrich@troll.no>
 *  Copyright (C) 2002 Aaron J. Seigo <aseigo@kde.org>
 *  Copyright (C) 2003 Nadeem Hasan <nhasan@kde.org>
 *  Copyright (C) 2004 Bernd Brandstetter <bbrand@freenet.de>
 *  Copyright (C) 2006 Urs Wolfer <uwolfer @ kde.org>
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

#ifndef KSNAPSHOT_H
#define KSNAPSHOT_H

#include <QAction>
#include <QTimer>
#include <QPixmap>
#include <QDialog>

#include "ksnapshotobject.h"
#include "snapshottimer.h"

#include "config-ksnapshot.h"

class KSnapshotWidget;
class QMenu;

class KSnapshot : public QDialog, public KSnapshotObject
{
  Q_OBJECT

public:
    explicit KSnapshot(QWidget *parent= 0, KSnapshotObject::CaptureMode mode = Region);
    ~KSnapshot();

public slots:
    void slotGrab();
    void slotCopy();
    void slotMovePointer( int x, int y );
    void setTime( int newTime );
    void setURL( const QString &newURL );
    void setGrabMode( int m );
    void exit();

protected:
    void reject() { close(); }
    virtual void closeEvent( QCloseEvent * e );
    void resizeEvent(QResizeEvent*);
    bool eventFilter( QObject*, QEvent* );
    virtual void refreshCaption();

private slots:
    void grabTimerDone();
    void slotDragSnapshot();
    void updateCaption();
    void updatePreview();
    void slotRegionGrabbed( const QPixmap & );
    void slotWindowGrabbed( const QPixmap & );
    void slotModeChanged( int mode );
    void setPreview( const QPixmap &pm );
    void setDelay( int i );
    void setIncludeDecorations( bool b );
    void setIncludePointer( bool b );
    bool includePointer() const;
    void setMode( int mode );
    int delay() const;
    bool includeDecorations() const;
    int mode() const;
    QPixmap preview();
    int previewWidth() const;
    int previewHeight() const;
    void startUndelayedGrab();
    void slotScreenshotReceived(qulonglong handle);
    void onOkBtnClicked();
    void onCancelBtnClicked();
    void onHelpBtnClicked();
    void onSaveBtnClicked();

public:
    int grabMode() const;
    int timeout() const;

private:
    void performGrab();
    void grabPointerImage(int offsetx, int offsety);
    void grabRegion();
    void grabFreeRegion();
    QString  getUnexistsFilePath(const QString &base);

    SnapshotTimer grabTimer;
    QTimer updateTimer;
    KSnapshotWidget *mainWidget;
    bool modified;
    QPoint savedPosition;
    bool haveXFixes;
};

#endif // KSNAPSHOT_H

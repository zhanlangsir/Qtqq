/*
 *  Copyright (C) 1997-2002 Richard J. Moore <rich@kde.org>
 *  Copyright (C) 2000 Matthias Ettrich <ettrich@troll.no>
 *  Copyright (C) 2002 Aaron J. Seigo <aseigo@kde.org>
 *  Copyright (C) 2003 Nadeem Hasan <nhasan@kde.org>
 *  Copyright (C) 2004 Bernd Brandstetter <bbrand@freenet.de>
 *  Copyright (C) 2006 Urs Wolfer <uwolfer @ kde.org>
 *  Copyright (C) 2007 Montel Laurent <montel@kde.org>
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

#ifndef KSNAPSHOTOBJECT_H
#define KSNAPSHOTOBJECT_H

#include <QUrl>
#include <QPixmap>

class QWidget;
class RegionGrabber;
class FreeRegionGrabber;
class QString;

class KSnapshotObject
{
public:
    enum CaptureMode { FullScreen=0, WindowUnderCursor=1, Region=2, FreeRegion=3, ChildWindow=4, CurrentScreen=5 };
     KSnapshotObject();
     virtual ~KSnapshotObject();

     bool saveImage( QIODevice *device, const QByteArray &format );

protected:
     void autoincFilename();
     virtual void refreshCaption(){}

     void changeUrl(const QString &newUrl);

     QString file_path_;
     RegionGrabber *rgnGrab;
     FreeRegionGrabber *freeRgnGrab;
     QWidget* grabber;
     QPixmap snapshot;
     QString title;
     QString windowClass;
};

#endif

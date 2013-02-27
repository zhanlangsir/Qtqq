/*
 *  Copyright (C) 1997-2008 Richard J. Moore <rich@kde.org>
 *  Copyright (C) 2000 Matthias Ettrich <ettrich@troll.no>
 *  Copyright (C) 2002 Aaron J. Seigo <aseigo@kde.org>
 *  Copyright (C) 2003 Nadeem Hasan <nhasan@kde.org>
 *  Copyright (C) 2004 Bernd Brandstetter <bbrand@freenet.de>
 *  Copyright (C) 2006-2008 Urs Wolfer <uwolfer @ kde.org>
 *  Copyright (C) 2007 Montel Laurent <montel@kde.org>
 *  Copyright (C) 2010 Pau Garcia i Quiles <pgquiles@elpauer.org>
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

#include "ksnapshotobject.h"

#include <QWidget>
#include <QRegExp>
#include <QApplication>
#include <QImageWriter>
#include <QFileInfo>
#include <QDebug>

KSnapshotObject::KSnapshotObject()
: rgnGrab( 0 ),
  freeRgnGrab( 0 ),
  grabber( 0 )
{
}

KSnapshotObject::~KSnapshotObject()
{
    delete grabber;
}

void KSnapshotObject::autoincFilename()
{
    // Extract the filename from the path
    QFileInfo info(file_path_);

    QString name= info.fileName();

    // If the name contains a number then increment it
    QRegExp numSearch( "(^|[^\\d])(\\d+)" ); // we want to match as far left as possible, and when the number is at the start of the name

    // Does it have a number?
    int start = numSearch.lastIndexIn( name );
    if (start != -1) {
        // It has a number, increment it
        start = numSearch.pos( 2 ); // we are only interested in the second group
        QString numAsStr = numSearch.capturedTexts()[ 2 ];
        QString number = QString::number( numAsStr.toInt() + 1 );
        number = number.rightJustified( numAsStr.length(), '0' );
        name.replace( start, numAsStr.length(), number );
    }
    else {
        // no number
        start = name.lastIndexOf('.');
        if (start != -1) {
            // has a . somewhere, e.g. it has an extension
            name.insert(start, '1');
        }
        else {
            // no extension, just tack it on to the end
            name += '1';
        }
    }

    //Rebuild the path
    QString newUrl = info.absolutePath() + '/' + name;
    changeUrl( newUrl );
}


void KSnapshotObject::changeUrl( const QString &url )
{
    if ( url == file_path_ )
        return;

    file_path_ = url;
    refreshCaption();
}

bool KSnapshotObject::saveImage( QIODevice *device, const QByteArray &format )
{
    QImageWriter imgWriter( device, format );

    if ( !imgWriter.canWrite() ) {
	qDebug() << "Cannot write format " << format;
	return false;
    }

    // For jpeg use 85% quality not the default
    if ( 0 == qstricmp(format.constData(), "jpeg") || 0 == qstricmp(format.constData(), "jpg") ) {
	imgWriter.setQuality( 85 );
    }

    if ( !title.isEmpty() )
	imgWriter.setText( QObject::tr("Title"), title );
    if ( !windowClass.isEmpty() )
	imgWriter.setText( QObject::tr("Window Class"), windowClass );

    QImage snap = snapshot.toImage();
    return imgWriter.write( snap );
}


/*
 *   Copyright (C) 2010 Pau Garcia i Quiles <pgquiles@elpauer.org>,
 *   based on the region grabber code by Luca Gugelmann <lucag@student.ethz.ch>
 *
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef FREEREGIONGRABBER_H
#define FREEREGIONGRABBER_H

#include <QWidget>
#include <QPoint>
#include <QRect>
#include <QPolygon>

class QPaintEvent;
class QMouseEvent;

class FreeRegionGrabber : public QWidget
{
    Q_OBJECT
    
public:
    FreeRegionGrabber();
    ~FreeRegionGrabber();

protected slots:
    void init();

signals:
    void freeRegionGrabbed( const QPixmap & );

protected:
    void paintEvent( QPaintEvent* e );
    void mousePressEvent( QMouseEvent* e );
    void mouseMoveEvent( QMouseEvent* e );
    void mouseReleaseEvent( QMouseEvent* e );
    void mouseDoubleClickEvent( QMouseEvent* );
    void keyPressEvent( QKeyEvent* e );
    void grabRect();

    QPolygon selection;
    bool mouseDown;
    bool newSelection;
    const int handleSize;
    QRect* mouseOverHandle;
    QPoint dragStartPoint;
    QPolygon  selectionBeforeDrag;
    bool showHelp;
    bool grabbing;

    QRect helpTextRect;

    QPixmap pixmap;
    QPoint pBefore;
};

#endif

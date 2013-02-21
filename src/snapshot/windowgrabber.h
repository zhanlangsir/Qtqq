/*
  Copyright (C) 2004 Bernd Brandstetter <bbrand@freenet.de>
  Copyright (C) 2010, 2011 Pau Garcia i Quiles <pgquiles@elpauer.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or ( at your option ) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifndef WINDOWGRABBER_H
#define WINDOWGRABBER_H

#include <QDialog>
#include <vector>

class WindowGrabber : public QDialog
{
    Q_OBJECT

public:
    WindowGrabber();
    ~WindowGrabber();

    /* Grab a screenshot of the current window.  x and y are set to the position of the window */
    static QPixmap grabCurrent( bool includeDecorations );
    static QString lastWindowTitle() { return WindowGrabber::title; }
    static QPoint lastWindowPosition() { return WindowGrabber::windowPosition; }

signals:
    void windowGrabbed( const QPixmap & );

protected:
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void wheelEvent( QWheelEvent * );
    void paintEvent( QPaintEvent * );

private:
    void increaseScope( const QPoint & );
    void decreaseScope( const QPoint & );
    int windowIndex( const QPoint & ) const;
    std::vector<QRect> windows;
    int current;
    int yPos;
    static QString title;
    static QPoint windowPosition;
};

#endif // WINDOWGRABBER_H

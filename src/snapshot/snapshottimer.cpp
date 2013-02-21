/*
 *  Copyright (C) 2007777777 Aaron J. Seigo <aseigo@kde.org>
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

#include "snapshottimer.h"
#include <QPainter>
#include <QPaintEvent>
#include <QToolTip>
#include <QApplication>
#include <QDesktopWidget>

#include <QDebug>

SnapshotTimer::SnapshotTimer()
    : QWidget(0),
      toggle(true)
{
    setWindowFlags( Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    // The text is copied from paintEvent and the maximum number is used as %1 argument + margins
    resize(fontMetrics().width(tr("Snapshot will be taken in 1 second",
                                     "Snapshot will be taken in %1 seconds", 99) ) + 6, fontMetrics().height() + 4);
    connect(&timer, SIGNAL(timeout()), this, SLOT(bell()));
}

SnapshotTimer::~SnapshotTimer()
{
}

void SnapshotTimer::start(int seconds)
{
    const QRect screenGeom = qApp->desktop()->screenGeometry();
    move(screenGeom.width() / 2 - size().width() / 2, screenGeom.top());
    toggle = true;
    time = 0;
    length = seconds;
    timer.start(1000);
    show();
}

void SnapshotTimer::stop()
{
    setVisible(false);
    hide();
    timer.stop();
}

void SnapshotTimer::bell()
{
    if (time == length - 1) {
        hide();
    }
    else {
        if (time == length) {
            emit timeout();
            timer.stop();
        }
    }
    ++time;
    toggle = !toggle;
    update();
}

void SnapshotTimer::enterEvent(QEvent *)
{
    const QRect screenGeom = qApp->desktop()->screenGeometry();
    if (x() == screenGeom.left()) {
        move(screenGeom.x() + (screenGeom.width() / 2 - size().width() / 2), screenGeom.top());
    }
    else {
        move(screenGeom.topLeft());
    }
}

void SnapshotTimer::paintEvent( QPaintEvent* e )
{
    Q_UNUSED( e );

    QPainter painter( this );

    if (time < length) {
      QPalette pal(QToolTip::palette());
      QColor handleColor = pal.color( QPalette::Active, QPalette::Highlight );
      handleColor.setAlpha( 160 );
      QColor overlayColor( 0, 0, 0, 160 );
      QColor textColor = pal.color( QPalette::Active, QPalette::Text );
      QColor textBackgroundColor = pal.color( QPalette::Active, QPalette::Base );
      if (toggle){
          textColor = pal.color( QPalette::Active, QPalette::Text);
      }
      else {
          textColor = pal.color( QPalette::Active, QPalette::Base);
      }
      painter.setPen( textColor );
      painter.setBrush( textBackgroundColor );
      const QString helpText = tr("Snapshot will be taken in 1 second",
                                     "Snapshot will be taken in %1 seconds", length - time);
      textRect = painter.boundingRect(rect().adjusted(2, 2, -2, -2), Qt::AlignHCenter | Qt::TextSingleLine, helpText);
      painter.drawText(textRect, helpText);
    }
}


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

#ifndef SNAPSHOTTIMER_H
#define SNAPSHOTTIMER_H

#include <QWidget>
#include <QTimer>

class SnapshotTimer : public QWidget
{
  Q_OBJECT

  public:
    SnapshotTimer();
    ~SnapshotTimer();
    void start(int seconds);
    void stop();

  signals:
    void timeout();

  protected slots:
    void bell();

  protected:
    void paintEvent(QPaintEvent *e);
    void enterEvent(QEvent *e);

  private:
    QTimer timer;
    QRect textRect;
    int time;
    int length;
    bool toggle;
};

#endif

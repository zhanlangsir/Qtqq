/*
 *  Copyright (C) 1997-2002 Richard J. Moore <rich@kde.org>
 *  Copyright (C) 2002-2010 Aaron J. Seigo <aseigo@kde.org>
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

#ifndef KSNAPSHOTPREVIEW_H
#define KSNAPSHOTPREVIEW_H

#include <QLabel>

class KSnapshotPreview : public QLabel
{
    Q_OBJECT

public:
    explicit KSnapshotPreview(QWidget *parent = 0);
    ~KSnapshotPreview();

    void setPreview(const QPixmap &pm);

signals:
    void startDrag();

protected:
    void mousePressEvent(QMouseEvent * e);
    void mouseMoveEvent(QMouseEvent * e);
    void mouseReleaseEvent(QMouseEvent * e);

private:
    QPoint mClickPt;
};

#endif // KSNAPSHOTPREVIEW_H

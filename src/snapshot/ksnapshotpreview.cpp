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

#include "ksnapshotpreview.h"

#include <QPainter>
#include <QMouseEvent>
#include <QPixmap>

#include <QDebug>

#include "expblur.cpp"

KSnapshotPreview::KSnapshotPreview(QWidget *parent)
    : QLabel(parent)
{
    setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    setCursor(Qt::OpenHandCursor);
}

KSnapshotPreview::~KSnapshotPreview()
{
}

void KSnapshotPreview::setPreview(const QPixmap &pm)
{
    static const int BLUR_PAD = 6;
    static const int BLUR_RADIUS = 2;
    QPixmap pixmap = pm.scaled(width() - BLUR_PAD, height() - BLUR_PAD, Qt::KeepAspectRatio, Qt::SmoothTransformation );

    QImage blur(pixmap.size() + QSize(BLUR_PAD, BLUR_PAD), QImage::Format_ARGB32);
    QRect blurRect = QRect(QPoint(BLUR_PAD / 2, BLUR_PAD / 2), pixmap.size());
    blur.fill(Qt::transparent);
    //kDebug() << blur.size() << blurRect << pixmap.size();

    const QColor color = qGray(palette().color(QPalette::Base).rgb()) < 192 ? Qt::white : Qt::black;

    {
        QPainter p(&blur);
        p.fillRect(blurRect, color);
        p.end();
    }

    // apply blur for the thumbnail shadow
    expblur<16, 7>(blur, BLUR_RADIUS);

    {
        QPainter p(&blur);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(blur.rect(), color);
        p.fillRect(QRect(blurRect.topLeft(), pixmap.size()), Qt::transparent);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.drawPixmap(QRect(blurRect.topLeft(), pixmap.size()), pixmap);
        p.end();
    }

    setPixmap(QPixmap::fromImage(blur));
}

void KSnapshotPreview::mousePressEvent(QMouseEvent * e)
{
    if ( e->button() == Qt::LeftButton )
        mClickPt = e->pos();
}

void KSnapshotPreview::mouseMoveEvent(QMouseEvent * e)
{
    if (mClickPt != QPoint(0, 0) &&
            (e->pos() - mClickPt).manhattanLength() > 200)
    {
        mClickPt = QPoint(0, 0);
        emit startDrag();
    }
}

void KSnapshotPreview::mouseReleaseEvent(QMouseEvent * /*e*/)
{
    mClickPt = QPoint(0, 0);
}

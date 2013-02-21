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

#include "freeregiongrabber.h"

#include <QPainter>
#include <QPaintEngine>
#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QToolTip>
#include <QTimer>

FreeRegionGrabber::FreeRegionGrabber( ) :
    QWidget( 0, Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool ),
    selection(), mouseDown( false ), newSelection( false ),
    handleSize( 10 ), mouseOverHandle( 0 ),
    showHelp( true ), grabbing( false )
{
    setMouseTracking( true );
    int timeout = 200;
    QTimer::singleShot( timeout, this, SLOT(init()) );
}

FreeRegionGrabber::~FreeRegionGrabber()
{
}

void FreeRegionGrabber::init()
{
    pixmap = QPixmap::grabWindow( QApplication::desktop()->winId() );
    resize( pixmap.size() );
    move( 0, 0 );
    setCursor( Qt::CrossCursor );
    show();
    grabMouse();
    grabKeyboard();
}

static void drawPolygon( QPainter *painter, const QPolygon &p, const QColor &outline, const QColor &fill = QColor() )
{
    QRegion clip( p );
    clip = clip - p;
    QPen pen(outline, 1, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin);

    painter->save();
    painter->setClipRegion( clip );
    painter->setPen(pen);
    painter->drawPolygon( p );
    if ( fill.isValid() ) {
        painter->setClipping( false );
        painter->setBrush( fill );
        painter->drawPolygon( p /*.translated( 1, -1 ) */);
    }
    painter->restore();
}

void FreeRegionGrabber::paintEvent( QPaintEvent* e )
{
    Q_UNUSED( e );
    if ( grabbing ) // grabWindow() should just get the background
        return;

    QPainter painter( this );

    QPalette pal(QToolTip::palette());
    QFont font = QToolTip::font();

    QColor handleColor = pal.color( QPalette::Active, QPalette::Highlight );
    handleColor.setAlpha( 160 );
    QColor overlayColor( 0, 0, 0, 160 );
    QColor textColor = pal.color( QPalette::Active, QPalette::Text );
    QColor textBackgroundColor = pal.color( QPalette::Active, QPalette::Base );
    painter.drawPixmap(0, 0, pixmap);
    painter.setFont(font);

    QPolygon pol = selection;
    if ( !selection.boundingRect().isNull() )
    {
        // Draw outline around selection.
        // Important: the 1px-wide outline is *also* part of the captured free-region because
        // I found no way to draw the outline *around* the selection because I found no way
        // to create a QPolygon which is smaller than the selection by 1px (QPolygon::translated
        // is NOT equivalent to QRect::adjusted)
        QPen pen(handleColor, 1, Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin);
        painter.setPen(pen);
        painter.drawPolygon( pol );

        // Draw the grey area around the selection
        QRegion grey( rect() );
        grey = grey - pol;
        painter.setClipRegion( grey );
        painter.setPen( Qt::NoPen );
        painter.setBrush( overlayColor );
        painter.drawRect( rect() );
        painter.setClipRect( rect() );
        drawPolygon( &painter, pol, handleColor);
    }

    if ( showHelp )
    {
        painter.setPen( textColor );
        painter.setBrush( textBackgroundColor );
        QString helpText = tr("Select a region using the mouse. To take the snapshot, press the Enter key or double click. Press Esc to quit.");
        helpTextRect = painter.boundingRect( rect().adjusted( 2, 2, -2, -2 ), Qt::TextWordWrap, helpText );
        helpTextRect.adjust( -2, -2, 4, 2 );
        drawPolygon( &painter, helpTextRect, textColor, textBackgroundColor );
        painter.drawText( helpTextRect.adjusted( 3, 3, -3, -3 ), helpText );
    }

    if ( selection.isEmpty() )
    {
        return;
    }

    // The grabbed region is everything which is covered by the drawn
    // rectangles (border included). This means that there is no 0px
    // selection, since a 0px wide rectangle will always be drawn as a line.
    QString txt = QString( "%1x%2" ).arg( selection.boundingRect().width() )
                  .arg( selection.boundingRect().height() );
    QRect textRect = painter.boundingRect( rect(), Qt::AlignLeft, txt );
    QRect boundingRect = textRect.adjusted( -4, 0, 0, 0);

    if ( textRect.width() < pol.boundingRect().width() - 2*handleSize &&
         textRect.height() < pol.boundingRect().height() - 2*handleSize &&
         ( pol.boundingRect().width() > 100 && pol.boundingRect().height() > 100 ) ) // center, unsuitable for small selections
    {
        boundingRect.moveCenter( pol.boundingRect().center() );
        textRect.moveCenter( pol.boundingRect().center() );
    }
    else if ( pol.boundingRect().y() - 3 > textRect.height() &&
              pol.boundingRect().x() + textRect.width() < rect().right() ) // on top, left aligned
    {
        boundingRect.moveBottomLeft( QPoint( pol.boundingRect().x(), pol.boundingRect().y() - 3 ) );
        textRect.moveBottomLeft( QPoint( pol.boundingRect().x() + 2, pol.boundingRect().y() - 3 ) );
    }
    else if ( pol.boundingRect().x() - 3 > textRect.width() ) // left, top aligned
    {
        boundingRect.moveTopRight( QPoint( pol.boundingRect().x() - 3, pol.boundingRect().y() ) );
        textRect.moveTopRight( QPoint( pol.boundingRect().x() - 5, pol.boundingRect().y() ) );
    }
    else if ( pol.boundingRect().bottom() + 3 + textRect.height() < rect().bottom() &&
              pol.boundingRect().right() > textRect.width() ) // at bottom, right aligned
    {
        boundingRect.moveTopRight( QPoint( pol.boundingRect().right(), pol.boundingRect().bottom() + 3 ) );
        textRect.moveTopRight( QPoint( pol.boundingRect().right() - 2, pol.boundingRect().bottom() + 3 ) );
    }
    else if ( pol.boundingRect().right() + textRect.width() + 3 < rect().width() ) // right, bottom aligned
    {
        boundingRect.moveBottomLeft( QPoint( pol.boundingRect().right() + 3, pol.boundingRect().bottom() ) );
        textRect.moveBottomLeft( QPoint( pol.boundingRect().right() + 5, pol.boundingRect().bottom() ) );
    }
    // if the above didn't catch it, you are running on a very tiny screen...
    drawPolygon( &painter, boundingRect, textColor, textBackgroundColor );

    painter.drawText( textRect, txt );

    if ( ( pol.boundingRect().height() > handleSize*2 && pol.boundingRect().width() > handleSize*2 )
         || !mouseDown )
    {
        painter.setBrush(QBrush(Qt::transparent));
        painter.setClipRegion( QRegion(pol));
        painter.drawPolygon( rect() );
    }
}

void FreeRegionGrabber::mousePressEvent( QMouseEvent* e )
{
    pBefore = e->pos();
    showHelp = !helpTextRect.contains( e->pos() );
    if ( e->button() == Qt::LeftButton )
    {
        mouseDown = true;
        dragStartPoint = e->pos();
        selectionBeforeDrag = selection;
        if ( !selection.containsPoint( e->pos(), Qt::WindingFill ) )
        {
            newSelection = true;
            selection = QPolygon();
        }
        else
        {
            setCursor( Qt::ClosedHandCursor );
        }
    }
    else if ( e->button() == Qt::RightButton )
    {
        newSelection = false;
        selection = QPolygon();
        setCursor( Qt::CrossCursor );
    }
    update();
}

void FreeRegionGrabber::mouseMoveEvent( QMouseEvent* e )
{
    bool shouldShowHelp = !helpTextRect.contains( e->pos() );
    if (shouldShowHelp != showHelp) {
        showHelp = shouldShowHelp;
        update();
    }

    if ( mouseDown )
    {
        if ( newSelection )
        {
            QPoint p = e->pos();
            selection << p;
        }
        else // moving the whole selection
        {
            QPoint p = e->pos() - pBefore; // Offset
            pBefore = e->pos(); // Save position for next iteration
            selection.translate(p);
        }

        update();
    }
    else
    {
        if ( selection.boundingRect().isEmpty() )
            return;

        if ( selection.containsPoint( e->pos(), Qt::WindingFill ) )
            setCursor( Qt::OpenHandCursor );
        else
            setCursor( Qt::CrossCursor );

    }
}

void FreeRegionGrabber::mouseReleaseEvent( QMouseEvent* e )
{
    mouseDown = false;
    newSelection = false;
    if ( mouseOverHandle == 0 && selection.containsPoint( e->pos(), Qt::WindingFill ) )
        setCursor( Qt::OpenHandCursor );
    update();
}

void FreeRegionGrabber::mouseDoubleClickEvent( QMouseEvent* )
{
    grabRect();
}

void FreeRegionGrabber::keyPressEvent( QKeyEvent* e )
{
    if ( e->key() == Qt::Key_Escape )
    {
        emit freeRegionGrabbed( QPixmap() );
    }
    else if ( e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return )
    {
        grabRect();
    }
    else
    {
        e->ignore();
    }
}

void FreeRegionGrabber::grabRect()
{
    QPolygon pol = selection;
    if ( !pol.isEmpty() )
    {
	grabbing = true;

        int xOffset = pixmap.rect().x() - pol.boundingRect().x();
        int yOffset = pixmap.rect().y() - pol.boundingRect().y();
        QPolygon translatedPol = pol.translated(xOffset, yOffset);

        QPixmap pixmap2(pol.boundingRect().size());
        pixmap2.fill(Qt::transparent);

        QPainter pt;
        pt.begin(&pixmap2);
        if (pt.paintEngine()->hasFeature(QPaintEngine::PorterDuff)) {
            pt.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform, true);
            pt.setBrush(Qt::black);
            pt.setPen(QPen(QBrush(Qt::black), 0.5));
            pt.drawPolygon(translatedPol);
            pt.setCompositionMode(QPainter::CompositionMode_SourceIn);
        } else {
            pt.setClipRegion(QRegion(translatedPol));
            pt.setCompositionMode(QPainter::CompositionMode_Source);
        }

        pt.drawPixmap(pixmap2.rect(), pixmap, pol.boundingRect());
        pt.end();

        emit freeRegionGrabbed(pixmap2);
    }
}


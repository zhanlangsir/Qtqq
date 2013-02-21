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

#include "windowgrabber.h"

#include <iostream>
#include <algorithm>

#include <QBitmap>
#include <QPainter>
#include <QPixmap>
#include <QPoint>
#include <QMouseEvent>
#include <QWheelEvent>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#ifdef HAVE_X11_EXTENSIONS_SHAPE_H
#include <X11/extensions/shape.h>
#endif // HAVE_X11_EXTENSIONS_SHAPE_H
#include <QX11Info>
#endif // Q_WS_X11

#ifdef Q_WS_WIN
#include <windows.h>

static UINT cxWindowBorder, cyWindowBorder;
#endif // Q_WS_WIN

static
const int minSize = 8;

static
bool operator< ( const QRect& r1, const QRect& r2 )
{
    return r1.width() * r1.height() < r2.width() * r2.height();
}

// Recursively iterates over the window w and its children, thereby building
// a tree of window descriptors. Windows in non-viewable state or with height
// or width smaller than minSize will be ignored.
#ifdef Q_WS_X11
static
void getWindowsRecursive( std::vector<QRect> *windows, Window w,
              int rx = 0, int ry = 0, int depth = 0 )
{
    XWindowAttributes atts;
    XGetWindowAttributes( QX11Info::display(), w, &atts );

    if ( atts.map_state == IsViewable &&
         atts.width >= minSize && atts.height >= minSize ) {
        int x = 0, y = 0;
        if ( depth ) {
            x = atts.x + rx;
            y = atts.y + ry;
        }

        QRect r( x, y, atts.width, atts.height );
        if ( std::find( windows->begin(), windows->end(), r ) == windows->end() ) {
            windows->push_back( r );
        }

        Window root, parent;
        Window* children;
        unsigned int nchildren;

        if( XQueryTree( QX11Info::display(), w, &root, &parent, &children, &nchildren ) != 0 ) {
                for( unsigned int i = 0; i < nchildren; ++i ) {
                    getWindowsRecursive( windows, children[ i ], x, y, depth + 1 );
                }

                if( children != NULL ) {
                    XFree( children );
                }
        }
    }

    if ( depth == 0 ) {
        std::sort( windows->begin(), windows->end() );
    }
}
#elif defined(Q_WS_WIN)
static
bool maybeAddWindow(HWND hwnd, std::vector<QRect> *windows) {
    WINDOWINFO wi;
    GetWindowInfo( hwnd, &wi );
    RECT rect = wi.rcClient;

#if 0
    RECT rect;
    GetWindowRect( hwnd, &rect );
#endif

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // For some reason, rect.left and rect.top are shifted by cxWindowBorders and cyWindowBorders pixels respectively 
    // in *every* case (for every window), but cxWindowBorders and cyWindowBorders are non-zero only in the 
    // biggest-window case, therefore we need to save the biggest cxWindowBorders and cyWindowBorders to adjust the rect later
    cxWindowBorder = qMax(cxWindowBorder, wi.cxWindowBorders);
    cyWindowBorder = qMax(cyWindowBorder, wi.cyWindowBorders);

    if ( ( ( wi.dwStyle & WS_VISIBLE ) != 0 ) && (width >= minSize ) && (height >= minSize ) )
    {
        //QRect r( rect.left + 4, rect.top + 4, width, height); // 4 = max(wi.cxWindowBorders) = max(wi.cyWindowBorders)
        QRect r(rect.left + cxWindowBorder, rect.top + cyWindowBorder, width, height); 
		if ( std::find( windows->begin(), windows->end(), r ) == windows->end() ) {
            windows->push_back( r );
            return true;
		}	
    }
    return false;
}

static
BOOL CALLBACK getWindowsRecursiveHelper( HWND hwnd, LPARAM lParam ) {
    maybeAddWindow(hwnd, reinterpret_cast< std::vector<QRect>* >(lParam) );
    return TRUE;
}

static
void getWindowsRecursive( std::vector<QRect> *windows, HWND hwnd,
              int rx = 0, int ry = 0, int depth = 0 )
{

    maybeAddWindow(hwnd, windows);

    EnumChildWindows( hwnd, getWindowsRecursiveHelper, (LPARAM) windows );

    std::sort( windows->begin(), windows->end() );
}
#endif // Q_WS_X11

#ifdef Q_WS_X11
static
Window findRealWindow( Window w, int depth = 0 )
{
    if( depth > 5 ) {
        return None;
    }

    static Atom wm_state = XInternAtom( QX11Info::display(), "WM_STATE", False );
    Atom type;
    int format;
    unsigned long nitems, after;
    unsigned char* prop;

    if( XGetWindowProperty( QX11Info::display(), w, wm_state, 0, 0, False, AnyPropertyType,
                            &type, &format, &nitems, &after, &prop ) == Success ) {
        if( prop != NULL ) {
            XFree( prop );
        }

        if( type != None ) {
            return w;
        }
    }

    Window root, parent;
    Window* children;
    unsigned int nchildren;
    Window ret = None;

    if( XQueryTree( QX11Info::display(), w, &root, &parent, &children, &nchildren ) != 0 ) {
        for( unsigned int i = 0;
             i < nchildren && ret == None;
             ++i ) {
            ret = findRealWindow( children[ i ], depth + 1 );
        }

        if( children != NULL ) {
            XFree( children );
        }
    }

    return ret;
}
#elif defined(Q_WS_WIN)
static
HWND findRealWindow( HWND w, int depth = 0 )
{
    // TODO Implement
    return w; // This is WRONG but makes code compile for now
}
#endif // Q_WS_X11

#ifdef Q_WS_X11
static
Window windowUnderCursor( bool includeDecorations = true )
{
    Window root;
    Window child;
    uint mask;
    int rootX, rootY, winX, winY;

    XGrabServer( QX11Info::display() );
    XQueryPointer( QX11Info::display(), QX11Info::appRootWindow(), &root, &child,
           &rootX, &rootY, &winX, &winY, &mask );

    if( child == None ) {
        child = QX11Info::appRootWindow();
    }

    if( !includeDecorations ) {
        Window real_child = findRealWindow( child );

        if( real_child != None ) { // test just in case
            child = real_child;
        }
    }

    return child;
}
#elif defined(Q_WS_WIN)
static
HWND windowUnderCursor(bool includeDecorations = true) 
{
    POINT pointCursor;
    QPoint qpointCursor = QCursor::pos();
    pointCursor.x = qpointCursor.x();
    pointCursor.y = qpointCursor.y();
    HWND windowUnderCursor = WindowFromPoint(pointCursor);

    if( includeDecorations ) {
        LONG_PTR style = GetWindowLongPtr( windowUnderCursor, GWL_STYLE );
        if( ( style & WS_CHILD ) != 0 ) {
            windowUnderCursor = GetAncestor( windowUnderCursor, GA_ROOT );
        }
    }
    return windowUnderCursor;
}
#endif

#ifdef Q_WS_X11
static
QPixmap grabWindow( Window child, int x, int y, uint w, uint h, uint border,
            QString *title=0, QString *windowClass=0 )
{
    QPixmap pm( QPixmap::grabWindow( QX11Info::appRootWindow(), x, y, w, h ) );

    if ( title ) {
        (*title) = "window";
    }

#ifdef HAVE_X11_EXTENSIONS_SHAPE_H
    int tmp1, tmp2;
    //Check whether the extension is available
    if ( XShapeQueryExtension( QX11Info::display(), &tmp1, &tmp2 ) ) {
    QBitmap mask( w, h );
    //As the first step, get the mask from XShape.
    int count, order;
    XRectangle* rects = XShapeGetRectangles( QX11Info::display(), child,
                         ShapeBounding, &count, &order );
    //The ShapeBounding region is the outermost shape of the window;
    //ShapeBounding - ShapeClipping is defined to be the border.
    //Since the border area is part of the window, we use bounding
    // to limit our work region
    if (rects) {
        //Create a QRegion from the rectangles describing the bounding mask.
        QRegion contents;
        for ( int pos = 0; pos < count; pos++ )
        contents += QRegion( rects[pos].x, rects[pos].y,
                     rects[pos].width, rects[pos].height );
        XFree( rects );

        //Create the bounding box.
        QRegion bbox( 0, 0, w, h );

        if( border > 0 ) {
            contents.translate( border, border );
            contents += QRegion( 0, 0, border, h );
            contents += QRegion( 0, 0, w, border );
            contents += QRegion( 0, h - border, w, border );
            contents += QRegion( w - border, 0, border, h );
        }

        //Get the masked away area.
        QRegion maskedAway = bbox - contents;
        QVector<QRect> maskedAwayRects = maskedAway.rects();

        //Construct a bitmap mask from the rectangles
        QPainter p(&mask);
        p.fillRect(0, 0, w, h, Qt::color1);
        for (int pos = 0; pos < maskedAwayRects.count(); pos++)
            p.fillRect(maskedAwayRects[pos], Qt::color0);
        p.end();

        pm.setMask(mask);
    }
    }
#endif // HAVE_X11_EXTENSIONS_SHAPE_H

    return pm;
}
#elif defined(Q_WS_WIN)
static
QPixmap grabWindow( HWND hWnd, QString *title=0, QString *windowClass=0 )
{
    RECT windowRect;
    GetWindowRect(hWnd, &windowRect);
    int w = windowRect.right - windowRect.left;
    int h = windowRect.bottom - windowRect.top;
    HDC targetDC = GetWindowDC(hWnd);
    HDC hDC = CreateCompatibleDC(targetDC);
    HBITMAP tempPict = CreateCompatibleBitmap(targetDC, w, h);
    HGDIOBJ oldPict = SelectObject(hDC, tempPict);
    BitBlt(hDC, 0, 0, w, h, targetDC, 0, 0, SRCCOPY);
    tempPict = (HBITMAP) SelectObject(hDC, oldPict);
    QPixmap pm = QPixmap::fromWinHBITMAP(tempPict);

    DeleteDC(hDC);
    ReleaseDC(hWnd, targetDC);

    if ( title ) {
        (*title) = "window";
    }

    return pm;
}
#endif // Q_WS_X11

QString WindowGrabber::title;
QPoint WindowGrabber::windowPosition;

WindowGrabber::WindowGrabber()
: QDialog( 0, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint ),
  current( -1 ), yPos( -1 )
{
    setWindowModality( Qt::WindowModal );
    int y,x;
    uint w, h;

#ifdef Q_WS_X11
    uint border, depth;
    Window root;
    XGrabServer( QX11Info::display() );
    Window child = windowUnderCursor();
    XGetGeometry( QX11Info::display(), child, &root, &x, &y, &w, &h, &border, &depth );
    XUngrabServer( QX11Info::display() );

    QPixmap pm( grabWindow( child, x, y, w, h, border, &title) );
#elif defined(Q_WS_WIN)
    HWND child = windowUnderCursor();
    
    WINDOWINFO wi;
    GetWindowInfo( child, &wi);

    RECT r;
    GetWindowRect( child, &r);
    x = r.left;
    y = r.top;
    w = r.right - r.left;
    h = r.bottom - r.top;
    cxWindowBorder = wi.cxWindowBorders;
    cyWindowBorder = wi.cyWindowBorders;

    HDC childDC = GetDC(child);

    QPixmap pm( grabWindow( child, &title) );
#endif // Q_WS_X11

    getWindowsRecursive( &windows, child );

    QPalette p = palette();
    p.setBrush( backgroundRole(), QBrush( pm ) );
    setPalette( p );
    setFixedSize( pm.size() );
    setMouseTracking( true );
    setGeometry( x, y, w, h );
    current = windowIndex( mapFromGlobal(QCursor::pos()) );
}

WindowGrabber::~WindowGrabber()
{
}

QPixmap WindowGrabber::grabCurrent( bool includeDecorations )
{
    int x, y;
#ifdef Q_WS_X11
    Window root;
    uint w, h, border, depth;

    XGrabServer( QX11Info::display() );
    Window child = windowUnderCursor( includeDecorations );
    XGetGeometry( QX11Info::display(), child, &root, &x, &y, &w, &h, &border, &depth );

    Window parent;
    Window* children;
    unsigned int nchildren;

    if( XQueryTree( QX11Info::display(), child, &root, &parent,
                    &children, &nchildren ) != 0 ) {
        if( children != NULL ) {
            XFree( children );
        }

        int newx, newy;
        Window dummy;

        if( XTranslateCoordinates( QX11Info::display(), parent, QX11Info::appRootWindow(),
            x, y, &newx, &newy, &dummy )) {
            x = newx;
            y = newy;
        }
    }

    windowPosition = QPoint(x,y);
    QPixmap pm( grabWindow( child, x, y, w, h, border, &title) );
    XUngrabServer( QX11Info::display() );
    return pm;
#elif defined(Q_WS_WIN)
    HWND hWindow;
    hWindow = windowUnderCursor(includeDecorations);
    Q_ASSERT(hWindow);

    HWND hParent;

// Now find the top-most window
    do {
      hParent = hWindow;
    } while( (hWindow = GetParent(hWindow)) != NULL );
    Q_ASSERT(hParent);

    RECT r;
    GetWindowRect(hParent, &r);

    x = r.left;
    y = r.top;

    windowPosition = QPoint(x,y);
    QPixmap pm( grabWindow( hParent, &title) );
    return pm;
#endif // Q_WS_X11
    return QPixmap();
}


void WindowGrabber::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() == Qt::RightButton ) {
        yPos = e->globalY();
    } else {
        if ( current != -1 ) {
            windowPosition = e->globalPos() - e->pos() + windows[current].topLeft();
            emit windowGrabbed( palette().brush( backgroundRole() ).texture().copy( windows[ current ] ) );
        } else {
            windowPosition = QPoint(0,0);
            emit windowGrabbed( QPixmap() );
        }
        accept();
    }
}

void WindowGrabber::mouseReleaseEvent( QMouseEvent *e )
{
    if ( e->button() == Qt::RightButton ) {
        yPos = -1;
    }
}

static
const int minDistance = 10;

void WindowGrabber::mouseMoveEvent( QMouseEvent *e )
{
    if ( yPos == -1 ) {
        int w = windowIndex( e->pos() );
        if ( w != -1 && w != current ) {
            current = w;
            repaint();
        }
    } else {
        int y = e->globalY();
        if ( y > yPos + minDistance ) {
            decreaseScope( e->pos() );
            yPos = y;
        } else if ( y < yPos - minDistance ) {
            increaseScope( e->pos() );
            yPos = y;
        }
    }
}

void WindowGrabber::wheelEvent( QWheelEvent *e )
{
    if ( e->delta() > 0 ) {
        increaseScope( e->pos() );
    } else if ( e->delta() < 0 ) {
        decreaseScope( e->pos() );
    } else {
        e->ignore();
    }
}

// Increases the scope to the next-bigger window containing the mouse pointer.
// This method is activated by either rotating the mouse wheel forwards or by
// dragging the mouse forwards while keeping the right mouse button pressed.
void WindowGrabber::increaseScope( const QPoint &pos )
{
    for ( uint i = current + 1; i < windows.size(); i++ ) {
        if ( windows[ i ].contains( pos ) ) {
            current = i;
            break;
        }
    }
    repaint();
}

// Decreases the scope to the next-smaller window containing the mouse pointer.
// This method is activated by either rotating the mouse wheel backwards or by
// dragging the mouse backwards while keeping the right mouse button pressed.
void WindowGrabber::decreaseScope( const QPoint &pos )
{
    for ( int i = current - 1; i >= 0; i-- ) {
    if ( windows[ i ].contains( pos ) ) {
        current = i;
        break;
    }
    }
    repaint();
}

// Searches and returns the index of the first (=smallest) window
// containing the mouse pointer.
int WindowGrabber::windowIndex( const QPoint &pos ) const
{
    for ( uint i = 0; i < windows.size(); i++ ) {
        if ( windows[ i ].contains( pos ) ) {
            return i;
        }
    }
    return -1;
}

// Draws a border around the (child) window currently containing the pointer
void WindowGrabber::paintEvent( QPaintEvent * )
{
    if ( current >= 0 ) {
        QPainter p;
        p.begin( this );
        p.fillRect(rect(), palette().brush( backgroundRole()));
        p.setPen( QPen( Qt::red, 3 ) );
        p.drawRect( windows[ current ].adjusted( 0, 0, -1, -1 ) );
        p.end();
    }
}

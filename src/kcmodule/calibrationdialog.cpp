/*
 * This file is part of the KDE wacomtablet project. For copyright
 * information and license terms see the AUTHORS and COPYING files
 * in the top-level directory of this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "calibrationdialog.h"
#include "debug.h"
//KDE includes
#include <KDE/KLocalizedString>

//Qt includes
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QX11Info>
#include <QtGui/QMessageBox>

// X11 includes
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>

using namespace Wacom;

const int debug = 0;
const int frameGap = 10;
const int boxwidth = 100;
const int numblocks = 8;

CalibrationDialog::CalibrationDialog( const QString &toolname, const QRect &screengeometry ) :
    QDialog( )
{
    setWindowState( Qt::WindowFullScreen );

    m_toolName = toolname;
    m_screenXOffset = screengeometry.x();
    m_screenYOffset = screengeometry.y();
    m_drawCross = 0;
    //m_shiftLeft = frameGap;
    //m_shiftTop = frameGap;
    m_shiftLeft = 0;
    m_shiftTop = 0;

    getMaxTabletArea();
    
    QLabel *showInfo = new QLabel();
    showInfo->setText( i18n( "Please tap into all four corners to calibrate the tablet.\nPress escape to cancel the process.") );
    showInfo->setAlignment( Qt::AlignCenter );
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget( showInfo );

    setLayout(mainLayout);

}

QRect CalibrationDialog::calibratedArea()
{
    return m_newtabletArea.toRect();
}

void CalibrationDialog::paintEvent( QPaintEvent *event )
{
    Q_UNUSED( event );

    QPainter painter( this );
    painter.setPen( Qt::black );

    if (m_shiftLeft == 0) {
        m_shiftLeft = size().width()/numblocks;
    }
    if (m_shiftTop == 0) {
        m_shiftTop = size().height()/numblocks;
    }
    
    // vertical line
    painter.drawLine( m_shiftLeft,
                      m_shiftTop - boxwidth/2,
                      m_shiftLeft,
                      m_shiftTop + boxwidth/2 );

    // horizontal line
    painter.drawLine( m_shiftLeft - boxwidth /2,
                      m_shiftTop ,
                      m_shiftLeft + boxwidth/2,
                      m_shiftTop );

    // draw circle around center
    painter.drawEllipse( QPoint( m_shiftLeft ,
                                 m_shiftTop ),
                         10, 10 );
}

void CalibrationDialog::mousePressEvent( QMouseEvent *event )
{
    if( event->pos().x() > m_shiftLeft - boxwidth/2
        && event->pos().x() < m_shiftLeft + boxwidth/2
        && event->pos().y() > m_shiftTop -boxwidth/2
        && event->pos().y() < m_shiftTop + boxwidth/2 ) {

        qreal delta_x = size().width()/numblocks;
        qreal delta_y = size().height()/numblocks;
    

        m_drawCross++;
        switch( m_drawCross ) {
        case 1:
            m_topLeft = event->globalPos();
            //m_shiftLeft = frameGap;
            //m_shiftTop = size().height() - frameGap - boxwidth;
            m_shiftTop = size().height() - delta_y -1 ;
            break;
        case 2:
            m_bottomLeft = event->globalPos();
            //m_shiftLeft = size().width() - frameGap - boxwidth;
            //m_shiftTop = size().height() - frameGap - boxwidth;
            m_shiftLeft = size().width() - delta_x - 1;
            break;
        case 3:
            m_bottomRight = event->globalPos();
            //m_shiftLeft = size().width() - frameGap - boxwidth;
            //m_shiftTop = frameGap;
            m_shiftTop = delta_y ;
            break;
        case 4:
            m_topRight = event->globalPos();
            calculateNewArea();
            close();
            break;
        }

        update();
    }
}

void CalibrationDialog::calculateNewArea()
{
    //qreal frameoffset = frameGeometry().height() - size().height();
    qreal tabletScreenRatioWidth = (m_originaltabletArea.width() - m_originaltabletArea.x())  / (float)size().width();
    qreal tabletScreenRatioHeight = (m_originaltabletArea.height() - m_originaltabletArea.y())/ (float)size().height();
    
    qreal clickedX = ( m_topLeft.x() + m_bottomLeft.x() ) / 2  - m_screenXOffset;
    qreal newX =  clickedX  * tabletScreenRatioWidth + m_originaltabletArea.x();

    qreal clickedY = ( m_topLeft.y() + m_topRight.y() ) / 2 - m_screenYOffset;
    qreal newY =  clickedY  * tabletScreenRatioHeight + m_originaltabletArea.y();

    qreal clickedWidth = ( m_topRight.x() + m_bottomRight.x() ) / 2 - m_screenXOffset;
    qreal newWidth =  clickedWidth  * tabletScreenRatioWidth + m_originaltabletArea.x();

    qreal clickedHeight = ( m_bottomRight.y() + m_bottomLeft.y() ) / 2 -  m_screenYOffset;
    qreal newHeight = clickedHeight * tabletScreenRatioHeight + m_originaltabletArea.y();
    
    /* add/subtract offset of calculated target points from corners */
    qreal delta_x = (newWidth - newX)/float(numblocks-2) ;
    qreal delta_y = (newHeight - newY)/float(numblocks-2);
    newX -=delta_x;
    newWidth +=delta_x;
    newY -=delta_y;
    newHeight +=delta_y;

    if (debug) {
        qDebug() << "OriginalXYmin: " << m_originaltabletArea.x() << "," << m_originaltabletArea.y();
        qDebug() << "OriginalXYmax: " << m_originaltabletArea.width() << "," << m_originaltabletArea.height();
        qDebug() << "ScreenSize: " << size().width() << "," << size().height();
        qDebug() << "RatioX: " << tabletScreenRatioWidth;
        qDebug() << "RatioY: " << tabletScreenRatioHeight;
        qDebug() << "clickedX: " << clickedX;
        qDebug() << "newX: "<< newX;
        qDebug() << "clickedY: " << clickedY;
        qDebug() << "newY: "<< newY;
        qDebug() << "clickedWidth" << clickedWidth;
        qDebug() << "newWidth: "<< newWidth;
        qDebug() << "clickedHeight" << clickedHeight;
        qDebug() << "newHeight: "<< newHeight;
        qDebug() << "delta_x: "<< delta_x;
        qDebug() << "delta_y: "<< delta_y;
    }
    
    m_newtabletArea.setX( newX );
    m_newtabletArea.setY( newY );
    // width and height values stored take origin XY into account, so subtract them
    m_newtabletArea.setWidth( newWidth - newX );
    m_newtabletArea.setHeight( newHeight -newY );
    
}

void CalibrationDialog::getMaxTabletArea()
{
    int ndevices;
    XDevice *dev = NULL;
    Display *dpy = QX11Info::display();

    XDeviceInfo *info = XListInputDevices( dpy, &ndevices );
    for( int i = 0; i < ndevices; i++ ) {
        if( info[i].name == m_toolName.toLatin1() ) {
            dev = XOpenDevice( dpy, info[i].id );
            break;
        }
    }

    Atom prop, type;
    int format;
    unsigned char *data = NULL;
    unsigned char *dataOld = NULL;
    unsigned long nitems, bytes_after;
    long *ldata;

    prop = XInternAtom( dpy, "Wacom Tablet Area", True );

    XGetDeviceProperty( dpy, dev, prop, 0, 1000, False, AnyPropertyType,
                        &type, &format, &nitems, &bytes_after, &dataOld );

    ldata = ( long * )dataOld;
    // load the original tablet area values 
    m_originaltabletArea.setX( ldata[0] );
    m_originaltabletArea.setY( ldata[1] );
    m_originaltabletArea.setWidth( ldata[2] );
    m_originaltabletArea.setHeight( ldata[3] );
    
    if (debug) {
        qDebug() << "originalX: "<< ldata[0];
        qDebug() << "originalY: "<< ldata[1];
        qDebug() << "originalWidth: "<< ldata[2];
        qDebug() << "originalHeight: "<< ldata[3];
    }

    XFlush( dpy );

    free( data );
    XFreeDeviceList( info );
    XCloseDevice( QX11Info::display(), dev );
}

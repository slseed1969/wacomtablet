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

#include "debug.h" // always needs to be first include

#include "areaselectionwidget.h"

#include <QtCore/QList>
#include <QtCore/QRect>
#include <QtCore/QString>

#include <QtGui/QBrush>
#include <QtGui/QCursor>
#include <QtGui/QFontMetrics>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPen>

using namespace Wacom;

namespace Wacom
{
    class AreaSelectionWidgetPrivate
    {
        public:
            AreaSelectionWidgetPrivate() {
                // set some reasonable default values
                dragMode                     = AreaSelectionWidget::DragNone;
                widgetTargetSize             = QSize(400,400);
                outOfBoundsMargin            = 0.;
                outOfBoundsVirtualAreaMargin = 0.;
                outOfBoundsDisplayAreaMargin = 0.;
                scaleFactor                  = .1; // prevent division by zero.

                drawAreaCaption              = true;
                drawSelectionCaption         = true;

                fontCaptions                 = QFont(QLatin1String("sans"), 10);

                // TODO make colors setable by the user
                colorDisplayAreaBrush        = QColor(Qt::lightGray);
                colorDisplayAreaPen          = QColor(Qt::black);
                colorDisplayAreaText         = QColor(Qt::black);

                colorSelectedAreaBrush       = QColor("#9EAEBF");
                colorSelectedAreaPen         = QColor("#555E67");
                colorSelectedAreaText        = QColor(colorSelectedAreaPen);

                colorDragHandles             = QColor(colorSelectedAreaPen);
            }

            static const qreal   DISPLAY_AREA_EXTRA_MARGIN; //!< An extra margin around the display area which is not taken into account for out of bounds calculations.
            static const qreal   DRAG_HANDLE_SIZE;          //!< The size of the drag handles.

            bool                 drawAreaCaption;        //!< Determines if the caption of the display areas should be drawn.
            bool                 drawSelectionCaption;   //!< Determines if the caption of the selected area should be drawn.

            QColor               colorDisplayAreaPen;    //!< The outline color of the display areas.
            QColor               colorDisplayAreaBrush;  //!< The fill colors of the display areas.
            QColor               colorDisplayAreaText;   //!< The text color of the display areas.

            QColor               colorDragHandles;       //!< The color used for the drag handles.

            QColor               colorSelectedAreaPen;   //!< The outline color of the selected area.
            QColor               colorSelectedAreaBrush; //!< The fill color of the selected area.
            QColor               colorSelectedAreaText;  //!< The text color of the selected area.

            QFont                fontCaptions;           //!< The font used for all captions.

            AreaSelectionWidget::DragMode dragMode;      //!< The current dragging mode if any.
            QPoint               dragPoint;              //!< The last mouse position while dragging the selected area.

            QSize                widgetTargetSize;       //!< The size of the widget we want, if possible.

            qreal                outOfBoundsMargin;      //!< The out of bounds margin as set by the user.
            qreal                outOfBoundsVirtualAreaMargin; //!< The number of real pixels (or a percentage) the user may drag the selected area outside of the virtual area.
            qreal                outOfBoundsDisplayAreaMargin; //!< The number of widget pixels which are calculated from the oob virtual area margin.

            qreal                scaleFactor;             //!< The scale factor which scales the virtual area's size to the widget's display area size.

            QList<QRect>         areaRectsList;           //!< The list of area rectangles which form the virtual area in real size.
            QStringList          areaCaptionsList;        //!< The list of captions for each area.

            QRect                rectVirtualArea;         //!< The rectangle which holds the virtual area in real size.

            QRectF               rectDisplayArea;         //!< The rectangle which holds the scaled virtual area displayed to the user.
            QList<QRectF>        rectDisplayAreas;        //!< The list of scaled sub-areas which make up the display area.
            QRectF               rectSelectedArea;        //!< The rectangle which holds the size and position of the selected display area.

            QRect                rectDragHandleTop;       //!< The rectangle which holds the size and position of the top drag handle.
            QRect                rectDragHandleRight;     //!< The rectangle which holds the size and position of the right drag handle.
            QRect                rectDragHandleBottom;    //!< The rectangle which holds the size and position of the bottom drag handle.
            QRect                rectDragHandleLeft;      //!< The rectangle which holds the size and position of the top drag handle.

    }; // PRIVATE CLASS

    const qreal AreaSelectionWidgetPrivate::DISPLAY_AREA_EXTRA_MARGIN = 5.;
    const qreal AreaSelectionWidgetPrivate::DRAG_HANDLE_SIZE          = 6.;

} // NAMESPACE


AreaSelectionWidget::AreaSelectionWidget(QWidget* parent)
        : QWidget(parent), d_ptr(new AreaSelectionWidgetPrivate)
{
}


AreaSelectionWidget::~AreaSelectionWidget()
{
    delete this->d_ptr;
}


void AreaSelectionWidget::clearSelection()
{
    Q_D(const AreaSelectionWidget);

    setSelection(d->rectVirtualArea);
}


const QRect AreaSelectionWidget::getSelection() const
{
    Q_D(const AreaSelectionWidget);

    return calculateUnscaledArea(d->rectSelectedArea, d->scaleFactor, getTotalDisplayAreaMargin());
}


const QString AreaSelectionWidget::getSelectionAsString() const
{
    QRect area = getSelection();
    return QString::fromLatin1("%1 %2 %3 %4").arg(area.x()).arg(area.y()).arg(area.width()).arg(area.height());
}


const QRect& AreaSelectionWidget::getVirtualArea() const
{
    Q_D(const AreaSelectionWidget);

    return d->rectVirtualArea;
}


void AreaSelectionWidget::setArea(const QRect& area, const QString& caption)
{
    QList< QRect > areaList;
    QStringList    captionList;

    areaList.append(area);
    captionList.append(caption);

    setAreas(areaList, captionList);
}


void AreaSelectionWidget::setAreas(const QList< QRect >& areas, const QStringList& areaCaptions)
{
    Q_D (AreaSelectionWidget);

    d->areaRectsList     = areas;
    d->areaCaptionsList  = areaCaptions;
    setupWidget();
}


void AreaSelectionWidget::setDrawAreaCaptions(bool value)
{
    Q_D(AreaSelectionWidget);

    d->drawAreaCaption = value;
}


void AreaSelectionWidget::setDrawSelectionCaption(bool value)
{
    Q_D(AreaSelectionWidget);

    d->drawSelectionCaption = value;
}


void AreaSelectionWidget::setFont(const QFont& font)
{
    Q_D(AreaSelectionWidget);

    d->fontCaptions = font;
}


void AreaSelectionWidget::setOutOfBoundsMargin(qreal margin)
{
    Q_D(AreaSelectionWidget);

    if (margin < 0.) {
        return;
    }

    d->outOfBoundsMargin = margin;
    setupWidget();
}


void AreaSelectionWidget::setSelection(const QRect& selection)
{
    Q_D(AreaSelectionWidget);

    // we can not select anything if we do not have areas to select from
    if (d->areaRectsList.isEmpty()) {
        return;
    }

    // if every value is -1 then the user wants to set the selection to the maximum
    QRect newSelection = selection;

    if (!selection.isValid() ||
        (selection.x() == -1 && selection.y() == -1 && selection.width() == -1 && selection.height() == -1)) {
        newSelection = d->rectVirtualArea;
    }

    // update selection and repaint widget
    d->rectSelectedArea = calculateScaledArea(newSelection, d->scaleFactor, getTotalDisplayAreaMargin());

    updateSelectedAreaSize();
    updateDragHandles();

    QWidget::update();
}


void AreaSelectionWidget::setSelection(int areaIndex)
{
    Q_D(const AreaSelectionWidget);

    if (areaIndex < 0 || areaIndex > d->areaRectsList.size()) {
        return;
    }

    setSelection(d->areaRectsList.at(areaIndex));
}



void AreaSelectionWidget::setWidgetTargetSize(const QSize& size)
{
    Q_D(AreaSelectionWidget);

    if (size.height() <= 0 || size.width() <= 0) {
        return;
    }

    d->widgetTargetSize = size;
    setupWidget();
}


void AreaSelectionWidget::mouseMoveEvent(QMouseEvent* event)
{
    Q_D(AreaSelectionWidget);

    // do nothing if we do not have areas set
    if (d->areaRectsList.isEmpty()) {
        return;
    }

    updateMouseCursor(event->pos());
    updateSelectedAreaOnDrag(event->pos());
    updateDragHandles();

    QWidget::update();
}


void AreaSelectionWidget::mousePressEvent(QMouseEvent* event)
{
    Q_D (AreaSelectionWidget);

    // do nothing if we do not have areas set or if the user is already dragging
    if (d->areaRectsList.isEmpty() || isUserDragging()) {
        return;
    }

    // determine what the user wants to drag - a handle or the whole selected area
    const QPoint mousePosition(event->pos());

    if (d->rectDragHandleTop.contains(mousePosition)){
        d->dragMode = AreaSelectionWidget::DragTopHandle;

    } else if (d->rectDragHandleRight.contains(mousePosition)) {
        d->dragMode = AreaSelectionWidget::DragRightHandle;

    } else if (d->rectDragHandleBottom.contains(mousePosition)) {
        d->dragMode = AreaSelectionWidget::DragBottomHandle;

    } else if (d->rectDragHandleLeft.contains(mousePosition)) {
        d->dragMode = AreaSelectionWidget::DragLeftHandle;

    } else if (d->rectSelectedArea.contains(mousePosition)) {
        d->dragMode  = AreaSelectionWidget::DragSelectedArea;
        d->dragPoint = mousePosition;

        QWidget::setCursor(Qt::SizeAllCursor);

    } else {
        // the user did not click anything that is dragable
        d->dragMode = AreaSelectionWidget::DragNone;
    }
}


void AreaSelectionWidget::mouseReleaseEvent(QMouseEvent* event)
{
    Q_D(AreaSelectionWidget);
    Q_UNUSED(event);

    // do nothing if we do not have areas set
    if (d->areaRectsList.isEmpty()) {
        return;
    }

    // if the user was dragging something, he is no longer now
    if (isUserDragging()) {
        d->dragMode = AreaSelectionWidget::DragNone;
        QWidget::setCursor(Qt::ArrowCursor);
        emit selectionChanged();
    }
}


void AreaSelectionWidget::paintEvent(QPaintEvent* event)
{
    Q_D(AreaSelectionWidget);
    Q_UNUSED(event);

    // draw nothing if we do not have areas set
    if (d->areaRectsList.isEmpty()) {
        QWidget::paintEvent(event);
        return;
    }

    QPainter painter( this );
    painter.setRenderHint( QPainter::Antialiasing );

    paintDisplayAreas(painter, false);
    paintSelectedArea(painter, false);
    paintDisplayAreas(painter, true);

    if (QWidget::isEnabled()) {
        paintDragHandles(painter);
    }

    if (d->drawAreaCaption) {
        paintDisplayAreaCaptions(painter);
    }

    if (d->drawSelectionCaption) {
        paintSelectedAreaCaption(painter);
    }
}


const QRectF AreaSelectionWidget::calculateDisplayArea(const QRect& virtualArea, qreal scaleFactor, qreal totalDisplayAreaMargin) const
{
    QRectF displayArea;

    displayArea.setX(totalDisplayAreaMargin);
    displayArea.setY(totalDisplayAreaMargin);
    displayArea.setWidth(virtualArea.width() * scaleFactor);
    displayArea.setHeight(virtualArea.height() * scaleFactor);

    return displayArea;
}


const QList< QRectF > AreaSelectionWidget::calculateDisplayAreas(const QList< QRect > areas, qreal scaleFactor, qreal totalDisplayAreaMargin) const
{
    QList<QRectF> displayAreas;
    QRectF        displayArea;

    foreach (QRect area, areas) {
        displayArea = calculateScaledArea(area, scaleFactor, totalDisplayAreaMargin);
        displayAreas.append(displayArea);
    }

    return displayAreas;
}


qreal AreaSelectionWidget::calculateOutOfBoundsVirtualAreaMargin(const QRect& virtualArea, qreal outOfBoundsMargin) const
{
    if (!virtualArea.isValid() || outOfBoundsMargin < 0.) {
        return 0.;
    }

    qreal margin = outOfBoundsMargin;

    if (outOfBoundsMargin <= 1.) {
        // out of bounds margin is a percentage - use the longer side of the virtual area as baseline
        if (virtualArea.width() > virtualArea.height()) {
            margin = outOfBoundsMargin * virtualArea.width();
        } else {
            margin = outOfBoundsMargin * virtualArea.height();
        }
    }

    return margin;
}


qreal AreaSelectionWidget::calculateScaleFactor(const QSize& targetSize, const QRect& virtualArea, qreal virtualAreaOutOfBoundsMargin, qreal displayAreaExtraMargin) const
{
    qreal scaleFactor = 0.1; // default is 10% to prevent a division by zero

    if (!virtualArea.isValid() || virtualArea.width() <= 0 || virtualArea.height() <= 0) {
        return scaleFactor;
    }

    if (virtualArea.width() > virtualArea.height()) {
        scaleFactor = (targetSize.width() - 2. * displayAreaExtraMargin) / (virtualArea.width() + 2. * virtualAreaOutOfBoundsMargin);

    } else {
        scaleFactor = (targetSize.height() - 2. * displayAreaExtraMargin) / (virtualArea.height() + 2. * virtualAreaOutOfBoundsMargin);
    }

    return scaleFactor;
}


const QRectF AreaSelectionWidget::calculateScaledArea(const QRect& area, qreal scaleFactor, qreal totalDisplayAreaMargin) const
{
    QRectF scaledArea;

    scaledArea.setX(area.x() * scaleFactor + totalDisplayAreaMargin);
    scaledArea.setY(area.y() * scaleFactor + totalDisplayAreaMargin);
    scaledArea.setWidth(area.width() * scaleFactor);
    scaledArea.setHeight(area.height() * scaleFactor);

    return scaledArea;
}


const QRect AreaSelectionWidget::calculateUnscaledArea(const QRectF& area, qreal scaleFactor, qreal totalDisplayAreaMargin) const
{
    QRect unscaledArea;

    unscaledArea.setX(qRound((area.x() - totalDisplayAreaMargin) / scaleFactor));
    unscaledArea.setY(qRound((area.y() - totalDisplayAreaMargin) / scaleFactor));
    unscaledArea.setWidth(qRound(area.width() / scaleFactor));
    unscaledArea.setHeight(qRound(area.height() / scaleFactor));

    return unscaledArea;
}


const QRect AreaSelectionWidget::calculateVirtualArea(const QList< QRect > areas) const
{
    QRect virtualArea;

    if (areas.length() == 1) {
        virtualArea = areas.at(0);

    } else if (areas.length() > 1) {
        for (int i = 0 ; i < areas.length() ; ++i) {
            virtualArea = virtualArea.united(areas.at(i));
        }
    }

    return virtualArea;
}


qreal AreaSelectionWidget::getTotalDisplayAreaMargin() const
{
    Q_D(const AreaSelectionWidget);

    return (d->outOfBoundsDisplayAreaMargin + d->DISPLAY_AREA_EXTRA_MARGIN);
}


bool AreaSelectionWidget::isUserDragging() const
{
    Q_D(const AreaSelectionWidget);

    return (d->dragMode != AreaSelectionWidget::DragNone);
}


void AreaSelectionWidget::paintDisplayAreaCaptions(QPainter& painter)
{
    Q_D(AreaSelectionWidget);

    QRectF       area;
    QString      caption;
    qreal        captionX;
    qreal        captionY;
    QFontMetrics fontMetrics(d->fontCaptions);

    painter.setPen( d->colorDisplayAreaText );
    painter.setBrush( d->colorDisplayAreaText );
    painter.setFont(d->fontCaptions);

    for (int i = 0 ; i < d->rectDisplayAreas.size() ; ++i) {
        area    = d->rectDisplayAreas.at(i);
        caption = (d->areaCaptionsList.size() > i) ? d->areaCaptionsList.at(i) : QString();

        if (!caption.isEmpty() && area.isValid()) {
            captionX = area.x() + (float)area.width() / 2 - (float)fontMetrics.width(caption) / 2;
            captionY = area.y() + (float)area.height() / 2 + (float)fontMetrics.height() / 2;

            painter.drawText(captionX, captionY, caption);
        }
    }
}


void AreaSelectionWidget::paintDisplayAreas(QPainter& painter, bool outlineOnly)
{
    Q_D(AreaSelectionWidget);

    // paint the whole display area
    painter.setPen( d->colorDisplayAreaPen );
    painter.setBrush( outlineOnly ? Qt::transparent : d->colorDisplayAreaBrush );

    if (d->rectDisplayAreas.size() > 1) {
        painter.drawRect( d->rectDisplayArea );
    }

    // paint the display sub areas and captions
    QRectF area;

    for (int i = 0 ; i < d->rectDisplayAreas.size() ; ++i) {
        area = d->rectDisplayAreas.at(i);

        if (area.isValid()) {
            painter.drawRect(area);
        }
    }
}


void AreaSelectionWidget::paintDragHandles(QPainter& painter)
{
    Q_D(AreaSelectionWidget);

    QColor color("#326583");

    painter.setPen( d->colorSelectedAreaPen );
    painter.setBrush( d->colorSelectedAreaPen );

    painter.drawRect(d->rectDragHandleTop);
    painter.drawRect(d->rectDragHandleRight);
    painter.drawRect(d->rectDragHandleBottom);
    painter.drawRect(d->rectDragHandleLeft);
}


void AreaSelectionWidget::paintSelectedArea(QPainter& painter, bool outlineOnly)
{
    Q_D(AreaSelectionWidget);

    painter.setPen( d->colorSelectedAreaPen );
    painter.setBrush(outlineOnly ? Qt::transparent : d->colorSelectedAreaBrush);
    painter.drawRect( d->rectSelectedArea );
}


void AreaSelectionWidget::paintSelectedAreaCaption(QPainter& painter)
{
    Q_D(AreaSelectionWidget);

    QFontMetrics fontMetrics(d->fontCaptions);

    painter.setPen(d->colorSelectedAreaText);
    painter.setBrush(d->colorSelectedAreaText);
    painter.setFont(d->fontCaptions);

    QRect selectedArea = getSelection();

    QString text = QString::fromLatin1("%1x%2+%3+%4")
                            .arg(selectedArea.width())
                            .arg(selectedArea.height())
                            .arg(selectedArea.x())
                            .arg(selectedArea.y());

    qreal textX = d->rectDisplayArea.x() + (float)d->rectDisplayArea.width() / 2 - (float)fontMetrics.width(text) / 2;
    qreal textY = d->rectDisplayArea.y() + (float)d->rectDisplayArea.height() / 2 + (float)fontMetrics.height() / 2;

    // If we draw area captions, then the caption already occupies
    // the vertical center. In this case we have to move our text a
    // bit further down.
    if (d->drawAreaCaption) {
        textY = textY + fontMetrics.height();
    }

    painter.drawText(qRound(textX), qRound(textY), text);
}



void AreaSelectionWidget::setupWidget()
{
    Q_D(AreaSelectionWidget);

    // do nothing if we do not have areas
    if (d->areaRectsList.isEmpty()) {
        return;
    }

    // calculate transformation data
    d->rectVirtualArea              = calculateVirtualArea(d->areaRectsList);
    d->outOfBoundsVirtualAreaMargin = calculateOutOfBoundsVirtualAreaMargin(d->rectVirtualArea, d->outOfBoundsMargin);
    d->scaleFactor                  = calculateScaleFactor(d->widgetTargetSize, d->rectVirtualArea, d->outOfBoundsVirtualAreaMargin, d->DISPLAY_AREA_EXTRA_MARGIN);
    d->outOfBoundsDisplayAreaMargin = d->outOfBoundsVirtualAreaMargin * d->scaleFactor;
    d->rectDisplayArea              = calculateDisplayArea(d->rectVirtualArea, d->scaleFactor, getTotalDisplayAreaMargin());
    d->rectDisplayAreas             = calculateDisplayAreas(d->areaRectsList, d->scaleFactor, getTotalDisplayAreaMargin());

    // setup widget
    QWidget::setMouseTracking( true );

    qreal widgetWidth  = d->rectDisplayArea.width() + 2. * getTotalDisplayAreaMargin();
    qreal widgetHeight = d->rectDisplayArea.height() + 2. * getTotalDisplayAreaMargin();

    QWidget::setMinimumSize(widgetWidth, widgetHeight);
    QWidget::setMaximumSize(widgetWidth, widgetHeight);

    // set selected area to the full display area
    d->rectSelectedArea = d->rectDisplayArea;

    // recalculate drag handles positions
    updateDragHandles();
}


void AreaSelectionWidget::updateDragHandles()
{
    Q_D (AreaSelectionWidget);

    // the size of the drag handles
    static const qreal handleSize     = d->DRAG_HANDLE_SIZE;
    static const qreal handleSizeHalf = handleSize / 2.;

    // some convenience vars so we have to type less
    const qreal areaX     = d->rectSelectedArea.x();
    const qreal areaY     = d->rectSelectedArea.y();
    const qreal areaW     = d->rectSelectedArea.width();
    const qreal areaWHalf = areaW / 2.;
    const qreal areaH     = d->rectSelectedArea.height();
    const qreal areaHHalf = areaH / 2.;

    d->rectDragHandleTop.setX(areaX + areaWHalf - handleSizeHalf);
    d->rectDragHandleTop.setY(areaY - handleSizeHalf);
    d->rectDragHandleTop.setWidth(handleSize);
    d->rectDragHandleTop.setHeight(handleSize);

    d->rectDragHandleRight.setX(areaX + areaW - handleSizeHalf);
    d->rectDragHandleRight.setY(areaY + areaHHalf - handleSizeHalf);
    d->rectDragHandleRight.setWidth(handleSize);
    d->rectDragHandleRight.setHeight(handleSize);

    d->rectDragHandleBottom.setX(areaX + areaWHalf - handleSizeHalf);
    d->rectDragHandleBottom.setY(areaY + areaH - handleSizeHalf);
    d->rectDragHandleBottom.setWidth(handleSize);
    d->rectDragHandleBottom.setHeight(handleSize);

    d->rectDragHandleLeft.setX(areaX - handleSizeHalf);
    d->rectDragHandleLeft.setY(areaY + areaHHalf - handleSizeHalf);
    d->rectDragHandleLeft.setWidth(handleSize);
    d->rectDragHandleLeft.setHeight(handleSize);
}


void AreaSelectionWidget::updateMouseCursor(const QPoint& mousePosition)
{
    Q_D(AreaSelectionWidget);

    // only update the cursor if the user is not dragging a handle.
    if (isUserDragging()) {
        return;
    }

    if (d->rectDragHandleLeft.contains(mousePosition) || d->rectDragHandleRight.contains(mousePosition)) {
        QWidget::setCursor( Qt::SizeHorCursor );

    } else if (d->rectDragHandleTop.contains(mousePosition) || d->rectDragHandleBottom.contains(mousePosition)) {
        QWidget::setCursor( Qt::SizeVerCursor );

    } else {
        QWidget::setCursor( Qt::ArrowCursor );
    }
}


void AreaSelectionWidget::updateSelectedAreaOnDrag(const QPoint& mousePosition)
{
    Q_D (AreaSelectionWidget);

    switch (d->dragMode) {

        case AreaSelectionWidget::DragTopHandle:
            updateSelectedAreaOnDragTop(mousePosition);
            break;

        case AreaSelectionWidget::DragRightHandle:
            updateSelectedAreaOnDragRight(mousePosition);
            break;

        case AreaSelectionWidget::DragBottomHandle:
            updateSelectedAreaOnDragBottom(mousePosition);
            break;

        case AreaSelectionWidget::DragLeftHandle:
            updateSelectedAreaOnDragLeft(mousePosition);
            break;

        case AreaSelectionWidget::DragSelectedArea:
            updateSelectedAreaOnDragArea(mousePosition);
            break;

        default:
            // prevent compiler warning
            break;
    }
}


void AreaSelectionWidget::updateSelectedAreaOnDragArea(const QPoint& mousePosition)
{
    Q_D (AreaSelectionWidget);

    const qreal dragXOffset = mousePosition.x() - d->dragPoint.x();
    const qreal dragYOffset = mousePosition.y() - d->dragPoint.y();

    const qreal leftBound   = d->rectDisplayArea.x() - d->outOfBoundsDisplayAreaMargin;
    const qreal rightBound  = d->rectDisplayArea.x() + d->rectDisplayArea.width() + d->outOfBoundsDisplayAreaMargin - d->rectSelectedArea.width();
    const qreal topBound    = d->rectDisplayArea.y() - d->outOfBoundsDisplayAreaMargin;
    const qreal bottomBound = d->rectDisplayArea.y() + d->rectDisplayArea.height() + d->outOfBoundsDisplayAreaMargin - d->rectSelectedArea.height();

    const qreal oldW = d->rectSelectedArea.width();
    const qreal oldH = d->rectSelectedArea.height();
    const qreal oldX = d->rectSelectedArea.x();
    const qreal oldY = d->rectSelectedArea.y();
    qreal       newX = oldX + dragXOffset;
    qreal       newY = oldY + dragYOffset;

    // use old values if the new ones are out of bounds
    if (newX < leftBound || rightBound < newX) {
        newX = oldX;
    }

    if (newY < topBound || bottomBound < newY) {
        newY = oldY;
    }

    // update selected area
    d->rectSelectedArea.setX(newX);
    d->rectSelectedArea.setY(newY);
    d->rectSelectedArea.setWidth(oldW);
    d->rectSelectedArea.setHeight(oldH);

    // update last drag point
    d->dragPoint = mousePosition;

    updateSelectedAreaSize(false);
}


void AreaSelectionWidget::updateSelectedAreaOnDragBottom(const QPoint& mousePosition)
{
    Q_D (AreaSelectionWidget);

    // the drag handle size in the bounds calculations ensure that the
    // box does not get too small to reach the drag handles with the mouse
    const qreal topBound    = d->rectSelectedArea.y() + d->DRAG_HANDLE_SIZE;
    const qreal bottomBound = d->rectDisplayArea.y() + d->rectDisplayArea.height() + d->outOfBoundsDisplayAreaMargin;
    const qreal mouseY      = mousePosition.y();
    qreal       newHeight   = 0.;

    if (mouseY < topBound) {
        newHeight = topBound - d->rectSelectedArea.y();

    } else if (mouseY > bottomBound) {
        newHeight = bottomBound - d->rectSelectedArea.y();

    } else {
        newHeight = mouseY - d->rectSelectedArea.y();
    }

    d->rectSelectedArea.setHeight(newHeight);

    updateSelectedAreaSize(true);
}


void AreaSelectionWidget::updateSelectedAreaOnDragLeft(const QPoint& mousePosition)
{
    Q_D (AreaSelectionWidget);

    // the drag handle size in the bounds calculations ensure that the
    // box does not get too small to reach the drag handles with the mouse
    const qreal leftBound  = d->rectDisplayArea.x() - d->outOfBoundsDisplayAreaMargin;
    const qreal rightBound = d->rectSelectedArea.x() + d->rectSelectedArea.width() - d->DRAG_HANDLE_SIZE;
    const qreal mouseX     = mousePosition.x();
    qreal       newX       = 0.;

    if (mouseX < leftBound) {
        newX = leftBound;

    } else if (mouseX > rightBound) {
        newX = rightBound;

    } else {
        newX = mouseX;
    }

    d->rectSelectedArea.setX(newX);

    updateSelectedAreaSize(false);
}


void AreaSelectionWidget::updateSelectedAreaOnDragRight(const QPoint& mousePosition)
{
    Q_D (AreaSelectionWidget);

    // the drag handle size in the bounds calculations ensure that the
    // box does not get too small to reach the drag handles with the mouse
    const qreal leftBound  = d->rectSelectedArea.x() + d->DRAG_HANDLE_SIZE;
    const qreal rightBound = d->rectDisplayArea.x() + d->rectDisplayArea.width() + d->outOfBoundsDisplayAreaMargin;
    const qreal mouseX     = mousePosition.x();
    qreal       newWidth   = 0.;

    if (mouseX < leftBound) {
        newWidth = leftBound - d->rectSelectedArea.x();

    } else if (mouseX > rightBound) {
        newWidth = rightBound - d->rectSelectedArea.x();

    } else {
        newWidth = mouseX - d->rectSelectedArea.x();
    }

    d->rectSelectedArea.setWidth(newWidth);

    updateSelectedAreaSize(true);
}


void AreaSelectionWidget::updateSelectedAreaOnDragTop(const QPoint& mousePosition)
{
    Q_D (AreaSelectionWidget);

    // the drag handle size in the bounds calculations ensure that the
    // box does not get too small to reach the drag handles with the mouse
    const qreal topBound    = d->rectDisplayArea.y() - d->outOfBoundsDisplayAreaMargin;
    const qreal bottomBound = d->rectSelectedArea.y() + d->rectSelectedArea.height() - d->DRAG_HANDLE_SIZE;
    const qreal mouseY      = mousePosition.y();
    qreal       newY        = 0.;

    if (mouseY < topBound) {
        newY = topBound;

    } else if (mouseY > bottomBound) {
        newY = bottomBound;

    } else {
        newY = mouseY;
    }

    d->rectSelectedArea.setY(newY);

    updateSelectedAreaSize(false);
}


void AreaSelectionWidget::updateSelectedAreaSize(bool fixPositionInsteadOfSize)
{
    Q_D(AreaSelectionWidget);

    // the selected area might be too large but
    // we expect it to be within valid bounds

    qreal selectedWidth  = d->rectSelectedArea.width();
    qreal selectedHeight = d->rectSelectedArea.height();
    qreal displayWidth   = d->rectDisplayArea.width();
    qreal displayHeight  = d->rectDisplayArea.height();

    if (selectedWidth > displayWidth) {

        if (fixPositionInsteadOfSize) {
            qreal newX = d->rectSelectedArea.x() + selectedWidth - displayWidth;
            d->rectSelectedArea.setX(newX);

        } else {
            d->rectSelectedArea.setWidth(displayWidth);
        }
    }

    if (selectedHeight > displayHeight) {

        if (fixPositionInsteadOfSize) {
            qreal newY = d->rectSelectedArea.y() + selectedHeight - displayHeight;
            d->rectSelectedArea.setY(newY);

        } else {
            d->rectSelectedArea.setHeight(displayHeight);
        }

    }
}

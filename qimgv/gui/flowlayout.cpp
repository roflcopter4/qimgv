/****************************************************************************
 **
 ** Copyright (C) 2016 The Qt Company Ltd.
 ** Contact: https://www.qt.io/licensing/
 **
 ** This file is part of the examples of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:BSD$
 ** Commercial License Usage
 ** Licensees holding valid commercial Qt licenses may use this file in
 ** accordance with the commercial license agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in
 ** a written agreement between you and The Qt Company. For licensing terms
 ** and conditions see https://www.qt.io/terms-conditions. For further
 ** information use the contact form at https://www.qt.io/contact-us.
 **
 ** BSD License Usage
 ** Alternatively, you may use this file under the terms of the BSD license
 ** as follows:
 **
 ** "Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions are
 ** met:
 **   * Redistributions of source code must retain the above copyright
 **     notice, this list of conditions and the following disclaimer.
 **   * Redistributions in binary form must reproduce the above copyright
 **     notice, this list of conditions and the following disclaimer in
 **     the documentation and/or other materials provided with the
 **     distribution.
 **   * Neither the name of The Qt Company Ltd nor the names of its
 **     contributors may be used to endorse or promote products derived
 **     from this software without specific prior written permission.
 **
 **
 ** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 ** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 ** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 ** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 ** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 ** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 ** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 ** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 ** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

#include "flowlayout.h"
#include <qmath.h>
#include <QWidget>

FlowLayout::FlowLayout()
{
    QSizePolicy sp = sizePolicy();
    sp.setHeightForWidth(true);
    setSizePolicy(sp);
}

qsizetype FlowLayout::itemAbove(qsizetype index) const
{
    if (index >= m_items.count() || index < 0)
        return -1;

    qsizetype indexAbove = index - m_columns;
    return indexAbove >= 0 ? indexAbove : index;
}

qsizetype FlowLayout::itemBelow(qsizetype index) const
{
    if (index >= m_items.count() || index < 0)
        return -1;

    if (sameRow(index, m_items.count() - 1))
        return index;

    qsizetype indexBelow = index + m_columns;

    return indexBelow < m_items.count() ? indexBelow
                                        : m_items.count() - 1;
}

bool FlowLayout::sameRow(qsizetype one, qsizetype two) const
{
    return one / m_columns == two / m_columns;
}

qsizetype FlowLayout::columnOf(qsizetype index) const
{
    if (index >= m_items.count() || index < 0)
        return -1;

    qsizetype col = index % m_columns;
    return col;
}

qsizetype FlowLayout::rows() const
{
    return m_rows;
}

qsizetype FlowLayout::columns() const
{
    return m_columns;
}

void FlowLayout::insertItem(qsizetype index, QGraphicsLayoutItem *item)
{
    item->setParentLayoutItem(this);
    if (index > m_items.count())
        index = m_items.count();
    m_items.insert(index, item);
    invalidate();
}

int FlowLayout::count() const
{
    return static_cast<int>(m_items.count());
}

QGraphicsLayoutItem *FlowLayout::itemAt(int index) const
{
    return m_items.value(index);
}

void FlowLayout::removeAt(int index)
{
    m_items.removeAt(index);
    invalidate();
}

void FlowLayout::clear()
{
    m_items.clear();
    invalidate();
}

qreal FlowLayout::spacing(Qt::Orientation o) const
{
    return m_spacing[static_cast<size_t>(o) - 1];
}

void FlowLayout::setSpacing(Qt::Orientations o, qreal spacing)
{
    if (o & Qt::Horizontal)
        m_spacing[0] = spacing;
    if (o & Qt::Vertical)
        m_spacing[1] = spacing;
}

void FlowLayout::setGeometry(QRectF const &geom)
{
    QGraphicsLayout::setGeometry(geom);
    GridInfo gInfo = doLayout(geom, true);
    m_columns      = gInfo.columns;
    m_rows         = gInfo.rows;
}

// this assumes every item is of the same size
FlowLayout::GridInfo FlowLayout::doLayout(QRectF const &geom, bool applyNewGeometry) const
{
    QElapsedTimer t;
    t.start();

    qreal leftMargin, topMargin, rightMargin, bottomMargin;
    getContentsMargins(&leftMargin, &topMargin, &rightMargin, &bottomMargin);

    qreal const maxRowWidth  = geom.width() - leftMargin - rightMargin;
    qsizetype   centerOffset = 0;

    qreal x = 0;
    qreal y = 0;

    QSizeF itemSize;

    qsizetype columns = m_items.count();
    qsizetype rows    = 0;
    if (columns)
        rows = 1;

    if (m_items.count()) {
        // calculate offset for centering
        qreal const itemWidth = m_items[0]->effectiveSizeHint(Qt::PreferredSize).width();
        auto const  maxCols   = static_cast<qsizetype>(maxRowWidth / itemWidth);

        if (m_items.count() >= maxCols)
            centerOffset = static_cast<qsizetype>(fmod(maxRowWidth, itemWidth) / 2);
        QGraphicsLayoutItem *item = m_items[0];
        itemSize                  = item->effectiveSizeHint(Qt::PreferredSize);
    }

    for (qsizetype i = 0; i < m_items.count(); ++i) {
        qreal next_x = x + itemSize.width();
        if (next_x > maxRowWidth) {
            if (x == 0) {
                itemSize.setWidth(maxRowWidth);
            } else {
                x      = 0;
                next_x = itemSize.width();
                if (rows == 1)
                    columns = i;
                rows++;
            }
            y += itemSize.height() + spacing(Qt::Vertical);
        }
        if (applyNewGeometry)
            m_items.at(i)->setGeometry(QRectF(QPointF(leftMargin + x + static_cast<qreal>(centerOffset), topMargin + y), itemSize));
        x = next_x + spacing(Qt::Horizontal);
    }

    // qDebug() << "elapsed: " << t.elapsed();
    return {columns, rows, topMargin + y + itemSize.height() + bottomMargin};
}

QSizeF FlowLayout::minSize(QSizeF const &constraint) const
{
    QSizeF size(0, 0);
    qreal  left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    if (constraint.width() >= 0) { // height for width
        qreal height = doLayout(QRectF(QPointF(0, 0), constraint), false).height;
        size         = QSizeF(constraint.width(), height);
    } else if (constraint.height() >= 0) { // width for height?
        // not supported
    } else {
        QGraphicsLayoutItem *item;
        Q_FOREACH (item, m_items)
            size = size.expandedTo(item->effectiveSizeHint(Qt::MinimumSize));
        size += QSize(static_cast<int>(left + right), static_cast<int>(top + bottom));
    }
    return size;
}

QSizeF FlowLayout::prefSize() const
{
    qreal left, right;
    getContentsMargins(&left, nullptr, &right, nullptr);

    QGraphicsLayoutItem *item;
    qreal maxh       = 0;
    qreal totalWidth = 0;

    Q_FOREACH (item, m_items) {
        if (totalWidth > 0)
            totalWidth += spacing(Qt::Horizontal);
        QSizeF pref = item->effectiveSizeHint(Qt::PreferredSize);
        totalWidth += pref.width();
        maxh = std::max(maxh, pref.height());
    }
    maxh += spacing(Qt::Vertical);

    static constexpr qreal goldenAspectRatio = 1.61803399;

    qreal w = qSqrt(totalWidth * maxh * goldenAspectRatio) + left + right;
    return minSize(QSizeF(w, -1));
}

QSizeF FlowLayout::maxSize() const
{
    QGraphicsLayoutItem *item;
    qreal totalWidth  = 0;
    qreal totalHeight = 0;

    Q_FOREACH (item, m_items) {
        if (totalWidth > 0)
            totalWidth += spacing(Qt::Horizontal);
        if (totalHeight > 0)
            totalHeight += spacing(Qt::Vertical);
        QSizeF pref = item->effectiveSizeHint(Qt::PreferredSize);
        totalWidth += pref.width();
        totalHeight += pref.height();
    }

    qreal left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);

    return {left + totalWidth + right, top + totalHeight + bottom};
}

QSizeF FlowLayout::sizeHint(Qt::SizeHint which, QSizeF const &constraint) const
{
    QSizeF sh;
    switch (which) {
    case Qt::PreferredSize: sh = prefSize();          break;
    case Qt::MinimumSize:   sh = minSize(constraint); break;
    case Qt::MaximumSize:   sh = maxSize();           break;
    default:                sh = constraint;          break;
    }
    return sh;
}

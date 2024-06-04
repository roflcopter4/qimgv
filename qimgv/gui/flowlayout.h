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

#pragma once

#include <QGraphicsLayout>
#include <QDebug>
#include <QElapsedTimer>

class FlowLayout final : public QGraphicsLayout
{
    struct GridInfo {
        qsizetype columns;
        qsizetype rows;
        qreal     height;
    };

  public:
    FlowLayout();

    void insertItem(qsizetype index, QGraphicsLayoutItem *item);
    void clear();

    // It's very annoying but there's no way around using `int` for these.
    ND int  count() const override;
    ND auto itemAt(int index) const -> QGraphicsLayoutItem * override;
       void removeAt(int index) override;
       void setGeometry(QRectF const &geom) override;

    // returns the index of item above / below
    ND qsizetype itemAbove(qsizetype index) const;
    ND qsizetype itemBelow(qsizetype index) const;
    ND qsizetype rows() const;
    ND qsizetype columns() const;
    ND qsizetype columnOf(qsizetype index) const;
    ND bool      sameRow(qsizetype one, qsizetype two) const;

    ND qreal spacing(Qt::Orientation o) const;
       void  setSpacing(Qt::Orientations o, qreal spacing);

  protected:
    ND QSizeF sizeHint(Qt::SizeHint which, QSizeF const &constraint = QSizeF()) const override;

  private:
    ND GridInfo doLayout(QRectF const &geom, bool applyNewGeometry) const;
    ND QSizeF   minSize(QSizeF const &constraint) const;
    ND QSizeF   prefSize() const;
    ND QSizeF   maxSize() const;

    QList<QGraphicsLayoutItem *> m_items;

    qreal     m_spacing[2] = {0.0, 0.0};
    qsizetype m_rows       = 0;
    qsizetype m_columns    = 0;
};

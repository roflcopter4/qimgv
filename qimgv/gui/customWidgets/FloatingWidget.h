/*
 * Base class for floating panels / overlay widgets.
 * It is not supposed to go into any kind of layout, just set parent & call show().
 * Usage: reimplement recalculateGeometry() method, which sets the new
 * geometry when the parent is resized.
 */

#pragma once

#include "gui/customWidgets/FloatingWidgetContainer.h"
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include <QWheelEvent>

class FloatingWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit FloatingWidget(FloatingWidgetContainer *parent);
    ~FloatingWidget() override = default;
    DELETE_COPY_MOVE_ROUTINES(FloatingWidget);

    enum class Position : uint8_t {
        Left,
        Right,
        Bottom,
        Top,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight,
        Center,
    };

    ND QSize containerSize() const;
    ND bool  acceptKeyboardFocus() const;
    void     setAcceptKeyboardFocus(bool mode);

  protected:
    // called whenever container rectangle changes
    // this does nothing, reimplement to use
    virtual void recalculateGeometry();

    void paintEvent(QPaintEvent *event) override;
    void setContainerSize(QSize newContainer);
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

  public Q_SLOTS:
    void hide();

  private Q_SLOTS:
    void onContainerResized(QSize size);

  private:
    // size of whatever widget we are overlayed on
    QSize   container;
    quint32 destructorCount      = 0;
    bool    mAcceptKeyboardFocus = false;
};

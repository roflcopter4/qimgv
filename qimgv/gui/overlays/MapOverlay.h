#pragma once

#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QWidget>

class MapOverlay : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(float opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(int y READ y WRITE setY)

  public:
    enum class Location {
        LeftTop,
        RightTop,
        RightBottom,
        LeftBottom,
    };

    explicit MapOverlay(QWidget *parent = nullptr);
    ~MapOverlay() override;

    void   resize(int size);
    ND int size() const;

    void animateVisible(bool visible);
    void enableVisibility(bool);

    void     setOpacity(qreal opacity);
    ND qreal opacity() const;

    void    setLocation(Location l);
    ND auto location() const -> Location;

    void   setMargin(int margin);
    ND int margin() const;

    void   setY(int y);
    ND int y() const;

    /**
     * @brief Updating navigation map
     * calculates outer(image) and inner(view area) squares.
     */
    void updateMap(QRectF const &drawingRect);

    /**
     * Recalculates map position on resize
     */
    void updatePosition();

    /**
     * Unit test functions
     */
    ND QSizeF inner() const;
    ND QSizeF outer() const;

  signals:
    void positionChanged(float x, float y);

  protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

  private:
    class MapOverlayPrivate;

    MapOverlayPrivate *d;
    bool visibilityEnabled;
    bool imageDoesNotFit;
};

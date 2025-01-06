#pragma once

#include "Settings.h"
#include "utils/ImageLib.h"
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QWidget>

class IconWidget : public QWidget
{
    enum class IconColorMode : uint8_t {
        CUSTOM,
        THEME,
        SOURCE,
    };

  public:
    explicit IconWidget(QWidget *parent = nullptr);
    ~IconWidget() override;
    DELETE_COPY_MOVE_ROUTINES(IconWidget);

    void  setIconPath(QString const &path);
    void  setIconOffset(int x, int y);
    void  setColorMode(IconColorMode _mode);
    void  setColor(QColor const &_color);

    ND QSize minimumSizeHint() const override;

  protected:
    void paintEvent(QPaintEvent *event) override;

  private Q_SLOTS:
    void onSettingsChanged();

  private:
    void loadIcon();
    void applyColor();

    QPixmap      *pixmap;
    QString       iconPath;
    QPoint        iconOffset;
    QColor        color;
    qreal         dpr             = devicePixelRatioF();
    qreal         pixmapDrawScale = 0.0;
    IconColorMode colorMode       = IconColorMode::THEME;
    bool          hiResPixmap     = false;
};

#pragma once

#include <QWidget>
#include <QStyleOption>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include "settings.h"
#include "utils/imagelib.h"

class IconWidget : public QWidget
{
    enum class IconColorMode {
        CUSTOM,
        THEME,
        SOURCE,
    };

  public:
    explicit IconWidget(QWidget *parent = nullptr);
    ~IconWidget() override;

    void  setIconPath(QString const &path);
    void  setIconOffset(int x, int y);
    void  setColorMode(IconColorMode _mode);
    void  setColor(QColor const &_color);

    ND QSize minimumSizeHint() const override;

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    void loadIcon();
    void applyColor();

    QString       iconPath;
    QColor        color;
    IconColorMode colorMode = IconColorMode::THEME;
    bool          hiResPixmap;
    QPoint        iconOffset;
    QPixmap      *pixmap;
    qreal         dpr, pixmapDrawScale;
};

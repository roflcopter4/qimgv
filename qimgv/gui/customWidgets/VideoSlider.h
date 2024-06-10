#pragma once

#include <QDebug>
#include <QMouseEvent>
#include <QSlider>
#include <QStyle>
#include <QStyleOptionSlider>

class VideoSlider : public QSlider
{
    Q_OBJECT

  public:
    explicit VideoSlider(QWidget *parent = nullptr);

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

  Q_SIGNALS:
    void sliderMovedX(int);

  private:
    void setValueAtCursor(QPoint pos);
};

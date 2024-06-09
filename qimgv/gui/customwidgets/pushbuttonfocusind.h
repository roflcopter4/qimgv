#pragma once
#include "Settings.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>

class PushButtonFocusInd : public QPushButton
{
    Q_OBJECT

  public:
    PushButtonFocusInd(QWidget *parent = nullptr);
    void setHighlighted(bool);
    bool isHighlighted() const;

  signals:
    void rightPressed();
    void rightClicked();

  protected:
    bool mIsHighlighted;

    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
};

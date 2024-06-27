#pragma once

#include <QApplication>
#include <QLabel>
#include <QBoxLayout>
#include <QScrollArea>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QPainter>
#include <QResizeEvent>
#include <cmath>

#include "gui/customWidgets/ThumbnailView.h"
#include "gui/customWidgets/ThumbnailWidget.h"
#include "sourcecontainers/Thumbnail.h"

class ThumbnailStrip final : public ThumbnailView
{
    Q_OBJECT

  public:
    explicit ThumbnailStrip(QWidget *parent);

    QSize itemSize();
    void  readSettings();

  private:
    static constexpr int thumbPadding = 9;
    int thumbMarginX = 2;
    int thumbMarginY = 4;

    void updateThumbnailPositions(qsizetype start, qsizetype end);
    void updateThumbnailPositions();
    void setupLayout();

    ThumbnailStyle mCurrentStyle;

  public Q_SLOTS:
    void focusOn(qsizetype index) override;
    void focusOnSelection() override;

  protected:
    void resizeEvent(QResizeEvent *event) override;
    void updateScrollbarIndicator() override;
    void addItemToLayout(ThumbnailWidget *widget, qsizetype pos) override;
    void removeItemFromLayout(qsizetype pos) override;
    void removeAll() override;
    auto createThumbnailWidget() -> ThumbnailWidget * override;
};
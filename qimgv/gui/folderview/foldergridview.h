#pragma once

#include <QGraphicsWidget>

#include "gui/customwidgets/thumbnailview.h"
#include "gui/customwidgets/thumbnailwidget.h"
#include "gui/flowlayout.h"
#include "utils/stuff.h"
#include "components/actionManager/ActionManager.h"

class FolderGridView : public ThumbnailView
{
    Q_OBJECT

  public:
    explicit FolderGridView(QWidget *parent = nullptr);
    ~FolderGridView() override;

    static constexpr int THUMBNAIL_SIZE_MIN = 80;  // px
    static constexpr int THUMBNAIL_SIZE_MAX = 400; // these should be divisible by ZOOM_STEP
    static constexpr int ZOOM_STEP          = 20;

    void selectAll();

  public Q_SLOTS:
    void show();
    void hide();

    void selectFirst();
    void selectLast();
    void pageUp();
    void pageDown();
    void selectAbove();
    void selectBelow();
    void selectNext();
    void selectPrev();

    void zoomIn();
    void zoomOut();
    void setThumbnailSize(int newSize);
    void setShowLabels(bool mode);
    void focusOn(qsizetype index) override;
    void focusOnSelection() override;
    void setDragHover(qsizetype index) override;

  private:
    void scrollToCurrent();

    FlowLayout     *flowLayout;
    QGraphicsWidget *holderWidget;
    qsizetype       shiftedCol;
    qsizetype       lastDragTarget = -1;

  private Q_SLOTS:
    void onitemSelected();

  protected:
    void resizeEvent(QResizeEvent *event) override;
    void updateScrollbarIndicator() override;
    void addItemToLayout(ThumbnailWidget *widget, qsizetype pos) override;
    void removeItemFromLayout(qsizetype pos) override;
    void removeAll() override;
    void setupLayout();
    void updateLayout() override;
    void fitSceneToContents() override;

    ThumbnailWidget *createThumbnailWidget() override;

    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    bool focusNextPrevChild(bool) override;

  Q_SIGNALS:
    void thumbnailSizeChanged(int);
};

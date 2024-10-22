#pragma once

#include "components/actionManager/ActionManager.h"
#include "gui/customWidgets/ThumbnailView.h"
#include "gui/customWidgets/ThumbnailWidget.h"
#include "gui/FlowLayout.h"
#include "utils/Stuff.h"
#include <QGraphicsWidget>

class FolderGridView : public ThumbnailView
{
    Q_OBJECT

  public:
    explicit FolderGridView(QWidget *parent = nullptr);
    ~FolderGridView() override = default;
    DELETE_COPY_MOVE_ROUTINES(FolderGridView);

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

  protected:
    void resizeEvent(QResizeEvent *event) override;
    void updateScrollbarIndicator() override;
    void addItemToLayout(ThumbnailWidget *widget, qsizetype pos) override;
    void removeItemFromLayout(qsizetype pos) override;
    void removeAll() override;
    void setupLayout();
    void updateLayout() override;
    void fitSceneToContents() override;
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    bool focusNextPrevChild(bool) override;

    ND auto createThumbnailWidget() -> ThumbnailWidget * override;

  private:
    void scrollToCurrent();

  Q_SIGNALS:
    void thumbnailSizeChanged(int);

  private Q_SLOTS:
    void onitemSelected();
    void onSettingsChanged();

  private:
    FlowLayout      *flowLayout;
    QGraphicsWidget *holderWidget;
    qsizetype        shiftedCol = -1;
    qsizetype        lastDragTarget = -1;
};
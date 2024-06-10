#pragma once

#include "gui/customWidgets/IconWidget.h"
#include "gui/customWidgets/SidePanelWidget.h"
#include "gui/customWidgets/SpinBoxInputFix.h"
#include "gui/overlays/CropOverlay.h"
#include <QAbstractItemView>
#include <QDebug>
#include <QPainter>
#include <QScreen>
#include <QStyleOption>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QWidget>
#include <QtGlobal>

namespace Ui {
class CropPanel;
}

class CropPanel : public SidePanelWidget
{
    Q_OBJECT

  public:
    explicit CropPanel(CropOverlay *_overlay, QWidget *parent = nullptr);
    ~CropPanel() override;

    void setImageRealSize(QSize);

  public Q_SLOTS:
    void onSelectionOutsideChange(QRect rect);
    void show() override;

  Q_SIGNALS:
    void crop(QRect);
    void cropAndSave(QRect);
    void cancel();
    void cropClicked();
    void selectionChanged(QRect);
    void selectAll();
    void aspectRatioChanged(QPointF);

  protected:
    void paintEvent(QPaintEvent *) override;
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

  private Q_SLOTS:
    void doCrop();
    void doCropSave();
    void onSelectionChange();
    void onAspectRatioChange();   // via manual input
    void onAspectRatioSelected(); // via ComboBox
    void setFocusCropBtn();
    void setFocusCropSaveBtn();

    void doCropDefaultAction();

  private:
    Ui::CropPanel *ui;
    QRect          cropRect;
    CropOverlay   *overlay;
    QSize          realSize;
};

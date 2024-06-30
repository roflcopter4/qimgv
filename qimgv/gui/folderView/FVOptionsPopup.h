#pragma once

#include "components/actionManager/ActionManager.h"
#include "gui/customWidgets/FloatingWidget.h"
#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include <QWidget>

namespace Ui {
class FVOptionsPopup;
}

class FVOptionsPopup : public QWidget
{
    Q_OBJECT

  public:
    explicit FVOptionsPopup(QWidget *parent = nullptr);
    ~FVOptionsPopup() override;
    DELETE_COPY_MOVE_ROUTINES(FVOptionsPopup);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void hideEvent(QHideEvent *event) override;

  private:
    void setViewMode(FolderViewMode mode);
    void setSimpleView();
    void setExtendedView();
    void setFoldersView();

  Q_SIGNALS:
    void dismissed();
    void viewModeSelected(FolderViewMode);

  public Q_SLOTS:
    void showAt(QPoint pos);

  private Q_SLOTS:
    void readSettings();
    void selectSimpleView();
    void selectExtendedView();
    void selectFoldersView();

  private:
    Ui::FVOptionsPopup *ui;
};

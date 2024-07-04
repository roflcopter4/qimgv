#pragma once

#include "Settings.h"
#include "components/actionManager/ActionManager.h"
#include "gui/customWidgets/OverlayWidget.h"
#include <QKeyEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>

namespace Ui {
class RenameOverlay;
}

class RenameOverlay : public OverlayWidget
{
    Q_OBJECT

  public:
    explicit RenameOverlay(FloatingWidgetContainer *parent);
    ~RenameOverlay() override;
    DELETE_COPY_MOVE_ROUTINES(RenameOverlay);

  protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void recalculateGeometry() override;

  private:
    void selectName();

  Q_SIGNALS:
    void renameRequested(QString name);

  public Q_SLOTS:
    void setName(QString const &name);
    void setBackdropEnabled(bool mode);
    void show();
    void hide();

  private Q_SLOTS:
    void rename();
    void onCancel();

  private:
    Ui::RenameOverlay *ui;
    QString            origName;
    QList<QString>     keyFilter;
    bool               backdrop = false;
};

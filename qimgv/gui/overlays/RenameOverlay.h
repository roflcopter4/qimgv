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

  public Q_SLOTS:
    void setName(QString const &name);
    void setBackdropEnabled(bool mode);
    void show();
    void hide();

  Q_SIGNALS:
    void renameRequested(QString name);

  protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void recalculateGeometry() override;

  private Q_SLOTS:
    void rename();
    void onCancel();

  private:
    Ui::RenameOverlay *ui;
    QString            origName;
    QList<QString>     keyFilter;
    bool               backdrop = false;

    void selectName();
};

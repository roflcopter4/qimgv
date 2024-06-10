#pragma once

#include "Settings.h"
#include "components/actionManager/ActionManager.h"
#include "gui/customWidgets/OverlayWidget.h"
#include <QKeyEvent>
#include <QLineEdit>
#include <QPushButton>
#include <QTextBrowser>
#include <QWheelEvent>
#include <QWidget>

namespace Ui {
class ChangelogWindow;
}

class ChangelogWindow : public OverlayWidget
{
    Q_OBJECT

  public:
    explicit ChangelogWindow(FloatingWidgetContainer *parent);
    ~ChangelogWindow() override;
    void setText(QString const &text);

  public Q_SLOTS:
    void show();
    void hide();

  protected:
    void paintEvent(QPaintEvent *) override;
    void wheelEvent(QWheelEvent *) override;
    void keyPressEvent(QKeyEvent *) override;

  private:
    Ui::ChangelogWindow *ui;

  private Q_SLOTS:
    void hideAndShutUp();
};

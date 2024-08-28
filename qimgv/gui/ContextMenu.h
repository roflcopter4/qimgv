#pragma once

#include "components/actionManager/ActionManager.h"
#include "components/scriptManager/ScriptManager.h"
#include <QPainter>
#include <QScreen>
#include <QStyleOption>
#include <QTimer>

namespace Ui {
class ContextMenu;
}

class ContextMenu : public QWidget
{
    Q_OBJECT

  public:
    explicit ContextMenu(QWidget *parent);
    ~ContextMenu() override;

    void setImageEntriesEnabled(bool mode);

  Q_SIGNALS:
    void showScriptSettings();

  public Q_SLOTS:
    void showAt(QPoint pos);
    void setGeometry(QRect geom);
    void show();
    void hide();

  private Q_SLOTS:
    void switchToMainPage();
    void switchToScriptsPage();

  protected:
    void mousePressEvent(QMouseEvent *) override;
    void paintEvent(QPaintEvent *) override;
    void keyPressEvent(QKeyEvent *event) override;

  private:
    void fillOpenWithMenu();

    Ui::ContextMenu *ui;
    QTimer           mTimer;
};

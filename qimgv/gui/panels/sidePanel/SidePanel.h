#pragma once

#include "gui/customWidgets/SidePanelWidget.h"
#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include <QVBoxLayout>
#include <QWidget>

namespace Ui {
class SidePanel;
}

class SidePanel : public QWidget
{
    Q_OBJECT

  public:
    explicit SidePanel(QWidget *parent = nullptr);
    ~SidePanel() override;

    void setWidget(SidePanelWidget *w);
    SidePanelWidget *widget() const;

  public slots:
    void show();
    void hide();

  protected:
    void paintEvent(QPaintEvent *) override;

  private:
    Ui::SidePanel   *ui;
    SidePanelWidget *mWidget;
};

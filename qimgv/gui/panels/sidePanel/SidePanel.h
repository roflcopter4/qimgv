#pragma once

#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include <QVBoxLayout>
#include <QWidget>
#include "gui/customwidgets/sidepanelwidget.h"

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

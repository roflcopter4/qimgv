#pragma once

#include <QObject>
#include <QWidget>

class SidePanelWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit SidePanelWidget(QWidget *parent = nullptr);

  Q_SIGNALS:

  public Q_SLOTS:
    virtual void show() = 0;
};

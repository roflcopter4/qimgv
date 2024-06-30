#pragma once

#include "gui/customWidgets/IconWidget.h"
#include <QMouseEvent>

class IconButton : public IconWidget
{
    Q_OBJECT

  public:
    explicit IconButton(QWidget *parent = nullptr);

    void setCheckable(bool mode);

    ND bool isChecked() const;
    ND bool isHovered() const = delete;

  public Q_SLOTS:
    void setChecked(bool mode);

  Q_SIGNALS:
    void clicked();
    void toggled(bool);

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

  private:
    bool mCheckable = false;
    bool mChecked   = false;
    bool mPressed   = false;
};

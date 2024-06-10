#pragma once

#include "gui/customWidgets/IconWidget.h"
#include <QMouseEvent>

class IconButton : public IconWidget {
    Q_OBJECT
    bool isHovered();
public:
    explicit IconButton(QWidget *parent = nullptr);
    void setCheckable(bool mode);
    bool isChecked() const;

public slots:
    void setChecked(bool mode);

signals:
    void clicked();
    void toggled(bool);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool mCheckable, mChecked, mPressed;
    void mouseMoveEvent(QMouseEvent *event) override;
};

#pragma once

#include "components/actionmanager/actionmanager.h"
#include "gui/customwidgets/iconbutton.h"

#include "Common.h"

enum class TriggerMode {
    Press,
    Click,
};

class ActionButton : public IconButton
{
  public:
    explicit ActionButton(QWidget *parent = nullptr);
    ActionButton(QString const &_actionName, QString const &_iconPath, QWidget *parent = nullptr);
    ActionButton(QString const &_actionName, QString const &_iconPath, int _size, QWidget *parent = nullptr);

    void setAction(QString const &_actionName);
    void setTriggerMode(TriggerMode mode);

    ND TriggerMode triggerMode() const;

  protected:
    void        mousePressEvent(QMouseEvent *event) override;
    void        mouseReleaseEvent(QMouseEvent *event) override;
    QString     actionName;
    TriggerMode mTriggerMode;
};

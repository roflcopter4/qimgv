// A menu item that executes an action on press

#pragma once

#include "components/actionManager/ActionManager.h"
#include "gui/customWidgets/MenuItem.h"

class ContextMenuItem : public MenuItem
{
    Q_OBJECT

  public:
    explicit ContextMenuItem(QWidget *parent = nullptr);
    ~ContextMenuItem() override;

    void setAction(QString const &action);

  Q_SIGNALS:
    void pressed();

  protected:
    void onPress() override;

  private:
    QString mAction;
};

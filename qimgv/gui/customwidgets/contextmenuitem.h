// A menu item that executes an action on press

#pragma once

#include "gui/customwidgets/menuitem.h"
#include "components/actionmanager/actionmanager.h"

class ContextMenuItem : public MenuItem {
    Q_OBJECT
public:
    ContextMenuItem(QWidget *parent = nullptr);
    ~ContextMenuItem() override;
    void setAction(QString const &mAction);

signals:
    void pressed();

protected:
    void onPress() override;

private:
    QString mAction;
};

#include "ContextMenuItem.h"

ContextMenuItem::ContextMenuItem(QWidget *parent)
    : MenuItem(parent)
{}

ContextMenuItem::~ContextMenuItem() = default;

void ContextMenuItem::setAction(QString const &action)
{
    mAction = action;
    setShortcutText(actionManager->shortcutForAction(mAction));
}

void ContextMenuItem::onPress()
{
    emit pressed();
    if (!mAction.isEmpty())
        actionManager->invokeAction(mAction);
}

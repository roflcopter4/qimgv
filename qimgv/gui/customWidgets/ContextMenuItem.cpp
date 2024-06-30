#include "ContextMenuItem.h"

ContextMenuItem::ContextMenuItem(QWidget *parent)
    : MenuItem(parent)
{}

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

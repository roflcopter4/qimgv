#include "actionbutton.h"

ActionButton::ActionButton(QWidget *parent)
    : IconButton(parent),
      mTriggerMode(TriggerMode::Click)
{
    this->setFocusPolicy(Qt::NoFocus);
    this->setProperty("checked", false);
}

ActionButton::ActionButton(QString const &actionName, QString const &iconPath, QWidget *parent) : ActionButton(parent)
{
    setIconPath(iconPath);
    setAction(actionName);
}

ActionButton::ActionButton(QString const &actionName, QString const &iconPath, int size, QWidget *parent)
    : ActionButton(actionName, iconPath, parent)
{
    if (size > 0)
        setFixedSize(size, size);
}

void ActionButton::setAction(QString const &newActionName)
{
    this->actionName = newActionName;
}

void ActionButton::setTriggerMode(TriggerMode mode)
{
    mTriggerMode = mode;
}

TriggerMode ActionButton::triggerMode() const
{
    return mTriggerMode;
}

void ActionButton::mousePressEvent(QMouseEvent *event)
{
    IconButton::mousePressEvent(event);
    if (mTriggerMode == TriggerMode::Press && event->button() == Qt::LeftButton)
        actionManager->invokeAction(actionName);
}

void ActionButton::mouseReleaseEvent(QMouseEvent *event)
{
    IconButton::mouseReleaseEvent(event);
    if (mTriggerMode == TriggerMode::Click && rect().contains(event->pos()) && event->button() == Qt::LeftButton)
        actionManager->invokeAction(actionName);
}

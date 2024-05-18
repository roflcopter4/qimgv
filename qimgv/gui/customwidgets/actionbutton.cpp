#include "actionbutton.h"

ActionButton::ActionButton(QWidget *parent)
    : IconButton(parent),
      mTriggerMode(TriggerMode::Click)
{
    this->setFocusPolicy(Qt::NoFocus);
    this->setProperty("checked", false);
}

ActionButton::ActionButton(QString const &_actionName, QString const &_iconPath, QWidget *parent)
    :  ActionButton(parent)
{
    setIconPath(_iconPath);
    setAction(_actionName);
}

ActionButton::ActionButton(QString const &_actionName, QString const &_iconPath, int _size, QWidget *parent)
    :  ActionButton(_actionName, _iconPath, parent)
{
    if(_size > 0)
        setFixedSize(_size, _size);
}

void ActionButton::setAction(QString const &_actionName) {
    actionName = _actionName;
}

void ActionButton::setTriggerMode(TriggerMode mode) {
    mTriggerMode = mode;
}

TriggerMode ActionButton::triggerMode() const
{
    return mTriggerMode;
}

void ActionButton::mousePressEvent(QMouseEvent *event) {
    IconButton::mousePressEvent(event);
    if(mTriggerMode == TriggerMode::Press && event->button() == Qt::LeftButton)
        actionManager->invokeAction(actionName);
}

void ActionButton::mouseReleaseEvent(QMouseEvent *event) {
    IconButton::mouseReleaseEvent(event);
    if(mTriggerMode == TriggerMode::Click && rect().contains(event->pos()) && event->button() == Qt::LeftButton)
        actionManager->invokeAction(actionName);

}

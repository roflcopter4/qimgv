#include "IconButton.h"

IconButton::IconButton(QWidget *parent)
    : IconWidget(parent),
      mCheckable(false),
      mChecked(false),
      mPressed(false)
{
}

void IconButton::setCheckable(bool mode) {
    mCheckable = mode;
}

bool IconButton::isChecked() const
{
    return mChecked;
}

void IconButton::setChecked(bool mode) {
    if(mCheckable && mode != mChecked) {
        mPressed = false;
        setProperty("pressed", false);
        mChecked = mode;
        setProperty("checked", mChecked);
        style()->unpolish(this);
        style()->polish(this);
    }
}

void IconButton::mousePressEvent([[maybe_unused]] QMouseEvent *event) {
    if(mCheckable) {
        setChecked(!this->property("checked").toBool());
        emit toggled(mChecked);
    } else {
        mPressed = true;
        setProperty("pressed", true);
    }
    style()->unpolish(this);
    style()->polish(this);
}

void IconButton::mouseReleaseEvent(QMouseEvent *event) {
    mPressed = false;
    if(rect().contains(event->pos()) && !mCheckable) {
        emit clicked();
    }
    if(!mChecked) {
        setProperty("pressed", false);
        style()->unpolish(this);
        style()->polish(this);
    }
}

void IconButton::mouseMoveEvent(QMouseEvent *event) {
    if(mChecked || !mPressed)
        return;
    if(rect().contains(event->pos())) {
        if(!property("pressed").toBool()) {
            setProperty("pressed", true);
            style()->unpolish(this);
            style()->polish(this);
        }
    } else {
        if(property("pressed").toBool()) {
            setProperty("pressed", false);
            style()->unpolish(this);
            style()->polish(this);
        }
    }
}

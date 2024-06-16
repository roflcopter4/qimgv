#include "SpinBoxInputFix.h"

SpinBoxInputFix::SpinBoxInputFix(QWidget *parent)
    : QSpinBox(parent)
{}

void SpinBoxInputFix::keyPressEvent(QKeyEvent *event)
{
    if (std::ranges::binary_search(allowedKeys, event->key()))
        QSpinBox::keyPressEvent(event);
    else
        event->ignore();
}

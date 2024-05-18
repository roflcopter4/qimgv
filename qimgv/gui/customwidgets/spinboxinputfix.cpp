#include "spinboxinputfix.h"

SpinBoxInputFix::SpinBoxInputFix(QWidget *parent) : QSpinBox(parent) {
    allowedKeys << QS("1")
                << QS("2")
                << QS("3")
                << QS("4")
                << QS("5")
                << QS("6")
                << QS("7")
                << QS("8")
                << QS("9")
                << QS("0")
                << QS("Up")
                << QS("Down")
                << QS("Left")
                << QS("Right")
                << QS("PgUp")
                << QS("PgDown")
                << QS("Enter")
                << QS("Home")
                << QS("End")
                << QS("Del")
                << QS("Backspace");
}

void SpinBoxInputFix::keyPressEvent(QKeyEvent *event) {
    quint32 nativeScanCode = event->nativeScanCode();
    QString key = actionManager->keyForNativeScancode(nativeScanCode);
    if(allowedKeys.contains(key)) {
        QSpinBox::keyPressEvent(event);
    } else {
        event->ignore();
    }
}

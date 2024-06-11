#include "KeySequenceEdit.h"

KeySequenceEdit::KeySequenceEdit(QWidget *parent)
    : QPushButton(parent)
{
    QPushButton::setText(tr("[Enter sequence]"));
}

QString KeySequenceEdit::sequence() const
{
    return mSequence;
}

void KeySequenceEdit::setText(QString const &text)
{
    mSequence = text;
    QPushButton::setText(mSequence);
}

void KeySequenceEdit::keyPressEvent(QKeyEvent *e)
{
    processEvent(e);
}

void KeySequenceEdit::mousePressEvent(QMouseEvent *e)
{
    processEvent(e);
}

void KeySequenceEdit::mouseReleaseEvent(QMouseEvent *e)
{
    processEvent(e);
}

void KeySequenceEdit::wheelEvent(QWheelEvent *e)
{
    processEvent(e);
}

// block native tab-switching so we can use it in shortcuts
bool KeySequenceEdit::focusNextPrevChild(bool)
{
    return false;
}

void KeySequenceEdit::processEvent(QEvent *e)
{
    auto *inputEvent = dynamic_cast<QInputEvent *>(e);
    if (!inputEvent)
        return;
    QString tmp = ShortcutBuilder::fromEvent(inputEvent);
    if (!tmp.isEmpty()) {
        mSequence = tmp;
        this->setText(mSequence);
        emit edited();
    }
}

#include "ShortcutBuilder.h"

QString ShortcutBuilder::fromEvent(QInputEvent *event)
{
    if (auto *keyEvent = dynamic_cast<QKeyEvent *>(event))
        return processKeyEvent(keyEvent);
    if (auto *wheelEvent = dynamic_cast<QWheelEvent *>(event))
        return processWheelEvent(wheelEvent);
    if (auto *mouseEvent = dynamic_cast<QMouseEvent *>(event))
        return processMouseEvent(mouseEvent);
    return {};
}

QString ShortcutBuilder::processWheelEvent(QWheelEvent *event)
{
    if (event->angleDelta() == QPoint(0, 0))
        return u""_s;

    QString sequence;
    if (event->angleDelta().ry() < 0)
        sequence = u"WheelDown"_s;
    else if (event->angleDelta().ry() > 0)
        sequence = u"WheelUp"_s;
    if (event->angleDelta().rx() < 0) // fallback to horizontal
        sequence = u"WheelDown"_s;
    else if (event->angleDelta().rx() > 0)
        sequence = u"WheelUp"_s;

    sequence.prepend(modifierKeys(event));
    return sequence;
}

// Detects mouse button clicks only
// DoubleClick works only for LMB
// Otherwise treated as a regular click
QString ShortcutBuilder::processMouseEvent(QMouseEvent *event)
{
    QString sequence;
    switch (event->button()) {
    case Qt::LeftButton:   sequence = u"LMB"_s;          break;
    case Qt::RightButton:  sequence = u"RMB"_s;          break;
    case Qt::MiddleButton: sequence = u"MiddleButton"_s; break;
    case Qt::XButton1:     sequence = u"XButton1"_s;     break;
    case Qt::XButton2:     sequence = u"XButton2"_s;     break;
    default:
        break;
    }
    sequence.prepend(modifierKeys(event));

    if (event->type() == QEvent::MouseButtonDblClick) {
        sequence.append(u"_DoubleClick"_s);
        return sequence;
    }
    if ((event->type() == QEvent::MouseButtonPress && event->button() != Qt::RightButton) ||
        (event->type() == QEvent::MouseButtonRelease && event->button() == Qt::RightButton))
    {
        return sequence;
    }

    return u""_s;
}

QString ShortcutBuilder::processKeyEvent(QKeyEvent *event)
{
    if (event->type() != QEvent::KeyPress || isModifier(Qt::Key(event->key())))
        return u""_s;
#if defined Q_OS_LINUX || defined Q_OS_WIN32
    return fromEventNativeScanCode(event);
#else
    return fromEventText(event);
#endif
}

QString ShortcutBuilder::modifierKeys(QInputEvent *event)
{
    QString     mods;
    auto const &map = InputMap::modifiers();
    for (auto const &[key, val] : map)
        if (event->modifiers().testFlag(val))
            mods.append(key + u'+');
    return mods;
#if 0
    QMapIterator<QString, Qt::KeyboardModifier> i(InputMap::modifiers());
    while(i.hasNext()) {
        i.next();
        if(event->modifiers().testFlag(i.value()))
            mods.append(i.key() + u"+"_s);
    }
    return mods;
#endif
}

bool ShortcutBuilder::isModifier(Qt::Key key)
{
    if (key == Qt::Key_Control || key == Qt::Key_Super_L || key == Qt::Key_Super_R || key == Qt::Key_AltGr ||
        key == Qt::Key_Shift || key == Qt::Key_Meta || key == Qt::Key_Alt) {
        return true;
    }
    return false;
}

QString ShortcutBuilder::fromEventNativeScanCode(QKeyEvent *event)
{
    // layout-independent method (mostly)
    // -------------------------
    // Still has some issues like when you use two layouts
    // where on the same button you have a letter on one layout and some symbol on the other.
    // I'mtx leaving this as-is because trying to fix all layouts will turn into a mess real quick.
    // You can always just add the same keybind using your alt. layout if it doesn't work.

    QString sequence = InputMap::keyName(event->nativeScanCode());
    if (sequence.isEmpty())
        return sequence;
    QString eventText = event->text();
    QChar   keyChr    = eventText.isEmpty() ? QChar() : eventText[0];

    // use alt characters accessed by shift (punctuation on the numbers row etc.)
    if (event->modifiers() == Qt::ShiftModifier && !(keyChr.isLetter() || !keyChr.isPrint() || keyChr.isSpace()))
        sequence = eventText;
    else if (!sequence.isEmpty())
        sequence.prepend(modifierKeys(event));

    // qDebug() << u"RESULT:"_s << sequence;
    return sequence;
}

QString ShortcutBuilder::fromEventText(QKeyEvent *event)
{
    // layout-dependent method
    // -------------------------
    // Works on platforms for which there is no native scancode support from Qt.
    // Keybinds will work only on the same layout they were added (except non-printables).
    QString sequence = QVariant::fromValue(Qt::Key(event->key())).toString();
    if (!sequence.isEmpty()) {
        // remove u"Key_"_s at the beginning
        sequence.remove(0, 4);
        // rename some keys to match the ones from inputmap
        // just a bandaid
        if (sequence == u"Return"_sv)
            sequence = u"Enter"_s;
        else if (sequence == u"Escape"_sv)
            sequence = u"Esc"_s;
    } else {
        // got an unknown key (usually something from non-eng layout)
        // use its text value instead
        sequence = QKeySequence(event->key()).toString();
    }
    if (!sequence.isEmpty())
        sequence.prepend(modifierKeys(event));
    return sequence;
}

#include "inputmap.h"

InputMap *inputMap = nullptr;

InputMap::InputMap() {
    initKeyMap();
    initModMap();
}

InputMap *InputMap::getInstance() {
    if(!inputMap) {
       inputMap = new InputMap();
    }
    return inputMap;
}

const QMap<quint32, QString> &InputMap::keys() {
    return keyMap;
}

const QMap<QString, Qt::KeyboardModifier> &InputMap::modifiers() {
    return modMap;
}

void InputMap::initKeyMap() {
    // key codes as reported by QKeyEvent::nativeScanCode()
    keyMap.clear();
#ifdef Q_OS_WIN32
    // windows keymap for qimgv

    // row 1
    keyMap.insert( 1 , QS("Esc") );
    keyMap.insert( 59 , QS("F1") );
    keyMap.insert( 60 , QS("F2") );
    keyMap.insert( 61 , QS("F3") );
    keyMap.insert( 62 , QS("F4") );
    keyMap.insert( 63 , QS("F5") );
    keyMap.insert( 64 , QS("F6") );
    keyMap.insert( 65 , QS("F7") );
    keyMap.insert( 66 , QS("F8") );
    keyMap.insert( 67 , QS("F9") );
    keyMap.insert( 68 , QS("F10") );
    keyMap.insert( 87 , QS("F11") );
    keyMap.insert( 88 , QS("F12") );
    //keyMap.insert(  , QS("Print") );
    keyMap.insert( 70 , QS("ScrollLock") );
    keyMap.insert( 69 , QS("Pause") );

    // row 2
    keyMap.insert( 41 , QS("`") );
    keyMap.insert( 2 , QS("1") );
    keyMap.insert( 3 , QS("2") );
    keyMap.insert( 4 , QS("3") );
    keyMap.insert( 5 , QS("4") );
    keyMap.insert( 6 , QS("5") );
    keyMap.insert( 7 , QS("6") );
    keyMap.insert( 8 , QS("7") );
    keyMap.insert( 9 , QS("8") );
    keyMap.insert( 10 , QS("9") );
    keyMap.insert( 11 , QS("0") );
    keyMap.insert( 12 , QS("-") );
    keyMap.insert( 13 , QS("=") );
    keyMap.insert( 14 , QS("Backspace") );
    keyMap.insert( 338 , QS("Ins") );
    keyMap.insert( 327 , QS("Home") );
    keyMap.insert( 329 , QS("PgUp") );

    // row 3
    keyMap.insert( 15 , QS("Tab") );
    keyMap.insert( 16 , QS("Q") );
    keyMap.insert( 17 , QS("W") );
    keyMap.insert( 18 , QS("E") );
    keyMap.insert( 19 , QS("R") );
    keyMap.insert( 20 , QS("T") );
    keyMap.insert( 21 , QS("Y") );
    keyMap.insert( 22 , QS("U") );
    keyMap.insert( 23 , QS("I") );
    keyMap.insert( 24 , QS("O") );
    keyMap.insert( 25 , QS("P") );
    keyMap.insert( 26 , QS("[") );
    keyMap.insert( 27 , QS("]") );
    keyMap.insert( 28 , QS("Enter") ); // Qt outputs QS("Return") here and QS("Enter") on numpad
    keyMap.insert( 339 , QS("Del") );
    keyMap.insert( 335 , QS("End") );
    keyMap.insert( 337 , QS("PgDown") );

    // row 4
    keyMap.insert( 58 , QS("CapsLock") );
    keyMap.insert( 30 , QS("A") );
    keyMap.insert( 31 , QS("S") );
    keyMap.insert( 32 , QS("D") );
    keyMap.insert( 33 , QS("F") );
    keyMap.insert( 34 , QS("G") );
    keyMap.insert( 35 , QS("H") );
    keyMap.insert( 36 , QS("J") );
    keyMap.insert( 37 , QS("K") );
    keyMap.insert( 38 , QS("L") );
    keyMap.insert( 39 , QS(";") );
    keyMap.insert( 40 , QS("'") );
    keyMap.insert( 43 , QS("\\") );

    // row 5
    keyMap.insert( 44 , QS("Z") );
    keyMap.insert( 45 , QS("X") );
    keyMap.insert( 46 , QS("C") );
    keyMap.insert( 47 , QS("V") );
    keyMap.insert( 48 , QS("B") );
    keyMap.insert( 49 , QS("N") );
    keyMap.insert( 50 , QS("M") );
    keyMap.insert( 51 , QS(",") );
    keyMap.insert( 52 , QS(".") );
    keyMap.insert( 53 , QS("/") );
    keyMap.insert( 328 , QS("Up") );

    // row 6
    keyMap.insert( 57 , QS("Space") );
    keyMap.insert( 349 , QS("Menu") );
    keyMap.insert( 331 , QS("Left") );
    keyMap.insert( 336 , QS("Down") );
    keyMap.insert( 333 , QS("Right") );

    // numpad
    keyMap.insert( 325 , QS("NumLock") );
    keyMap.insert( 309 , QS("/") );
    keyMap.insert( 55 , QS("*") );
    keyMap.insert( 74 , QS("-") );
    keyMap.insert( 71 , QS("7") );
    keyMap.insert( 72 , QS("8") );
    keyMap.insert( 73 , QS("9") );
    keyMap.insert( 78 , QS("+") );
    keyMap.insert( 75 , QS("4") );
    keyMap.insert( 76 , QS("5") );
    keyMap.insert( 77 , QS("6") );
    keyMap.insert( 79 , QS("1") );
    keyMap.insert( 80 , QS("2") );
    keyMap.insert( 81 , QS("3") );
    keyMap.insert( 284 , QS("Enter") );
    keyMap.insert( 82 , QS("0") );
    keyMap.insert( 83 , QS(".") );

    // special
    //keyMap.insert( ?? , "Wake Up" ); // "Fn" key on thinkpad
    keyMap.insert( 86 , "<" ); // near left shift (iso layout)
    //keyMap.insert(??, "PgBack");
    //keyMap.insert(??, "PgForward");

    // looks like qt 6.7.0 changed nativeScanCode() values on windows
    // see https://github.com/easymodo/qimgv/issues/539
    keyMap.insert( 57426 , "Ins" );
    keyMap.insert( 57415 , "Home" );
    keyMap.insert( 57417 , "PgUp" );
    keyMap.insert( 57427 , "Del" );
    keyMap.insert( 57423 , "End" );
    keyMap.insert( 57425 , "PgDown" );
    keyMap.insert( 57416 , "Up" );
    keyMap.insert( 57437 , "Menu" );
    keyMap.insert( 57419 , "Left" );
    keyMap.insert( 57424 , "Down" );
    keyMap.insert( 57421 , "Right" );
    // numpad
    keyMap.insert( 57413 , "NumLock" );
    keyMap.insert( 57397 , "/" );
    keyMap.insert( 57372 , "Enter" );

#elif defined __linux__
    // linux keymap for qimgv

    // row 1
    keyMap.insert( 9 , QS("Esc") );
    keyMap.insert( 67 , QS("F1") );
    keyMap.insert( 68 , QS("F2") );
    keyMap.insert( 69 , QS("F3") );
    keyMap.insert( 70 , QS("F4") );
    keyMap.insert( 71 , QS("F5") );
    keyMap.insert( 72 , QS("F6") );
    keyMap.insert( 73 , QS("F7") );
    keyMap.insert( 74 , QS("F8") );
    keyMap.insert( 75 , QS("F9") );
    keyMap.insert( 76 , QS("F10") );
    keyMap.insert( 95 , QS("F11") );
    keyMap.insert( 96 , QS("F12") );
    keyMap.insert( 107 , QS("Print") );
    keyMap.insert( 78 , QS("ScrollLock") );
    keyMap.insert( 127 , QS("Pause") );

    // row 2
    keyMap.insert( 49 , QS("`") );
    keyMap.insert( 10 , QS("1") );
    keyMap.insert( 11 , QS("2") );
    keyMap.insert( 12 , QS("3") );
    keyMap.insert( 13 , QS("4") );
    keyMap.insert( 14 , QS("5") );
    keyMap.insert( 15 , QS("6") );
    keyMap.insert( 16 , QS("7") );
    keyMap.insert( 17 , QS("8") );
    keyMap.insert( 18 , QS("9") );
    keyMap.insert( 19 , QS("0") );
    keyMap.insert( 20 , QS("-") );
    keyMap.insert( 21 , QS("=") );
    keyMap.insert( 22 , QS("Backspace") );
    keyMap.insert( 118 , QS("Ins") );
    keyMap.insert( 110 , QS("Home") );
    keyMap.insert( 112 , QS("PgUp") );

    // row 3
    keyMap.insert( 23 , QS("Tab") );
    keyMap.insert( 24 , QS("Q") );
    keyMap.insert( 25 , QS("W") );
    keyMap.insert( 26 , QS("E") );
    keyMap.insert( 27 , QS("R") );
    keyMap.insert( 28 , QS("T") );
    keyMap.insert( 29 , QS("Y") );
    keyMap.insert( 30 , QS("U") );
    keyMap.insert( 31 , QS("I") );
    keyMap.insert( 32 , QS("O") );
    keyMap.insert( 33 , QS("P") );
    keyMap.insert( 34 , QS("[") );
    keyMap.insert( 35 , QS("]") );
    keyMap.insert( 36 , QS("Enter") ); // Qt outputs QS("Return") here and QS("Enter") on numpad
    keyMap.insert( 119 , QS("Del") );
    keyMap.insert( 115 , QS("End") );
    keyMap.insert( 117 , QS("PgDown") );

    // row 4
    keyMap.insert( 66 , QS("CapsLock") );
    keyMap.insert( 38 , QS("A") );
    keyMap.insert( 39 , QS("S") );
    keyMap.insert( 40 , QS("D") );
    keyMap.insert( 41 , QS("F") );
    keyMap.insert( 42 , QS("G") );
    keyMap.insert( 43 , QS("H") );
    keyMap.insert( 44 , QS("J") );
    keyMap.insert( 45 , QS("K") );
    keyMap.insert( 46 , QS("L") );
    keyMap.insert( 47 , QS(";") );
    keyMap.insert( 48 , QS("'") );
    keyMap.insert( 51 , QS("\\") );

    // row 5
    keyMap.insert( 52 , QS("Z") );
    keyMap.insert( 53 , QS("X") );
    keyMap.insert( 54 , QS("C") );
    keyMap.insert( 55 , QS("V") );
    keyMap.insert( 56 , QS("B") );
    keyMap.insert( 57 , QS("N") );
    keyMap.insert( 58 , QS("M") );
    keyMap.insert( 59 , QS(",") );
    keyMap.insert( 60 , QS(".") );
    keyMap.insert( 61 , QS("/") );
    keyMap.insert( 111 , QS("Up") );

    // row 6
    keyMap.insert( 65 , QS("Space") );
    keyMap.insert( 135 , QS("Menu") );
    keyMap.insert( 113 , QS("Left") );
    keyMap.insert( 116 , QS("Down") );
    keyMap.insert( 114 , QS("Right") );

    // numpad
    keyMap.insert( 77 , QS("NumLock") );
    keyMap.insert( 106 , QS("/") );
    keyMap.insert( 63 , QS("*") );
    keyMap.insert( 82 , QS("-") );
    keyMap.insert( 79 , QS("7") );
    keyMap.insert( 80 , QS("8") );
    keyMap.insert( 81 , QS("9") );
    keyMap.insert( 86 , QS("+") );
    keyMap.insert( 83 , QS("4") );
    keyMap.insert( 84 , QS("5") );
    keyMap.insert( 85 , QS("6") );
    keyMap.insert( 87 , QS("1") );
    keyMap.insert( 88 , QS("2") );
    keyMap.insert( 89 , QS("3") );
    keyMap.insert( 104 , QS("Enter") );
    keyMap.insert( 90 , QS("0") );
    keyMap.insert( 91 , QS(".") );

    // special
    keyMap.insert( 151 , QS("Wake Up") ); // QS("Fn") key on thinkpad
    keyMap.insert( 94 , QS("<") ); // near left shift (iso layout)
    keyMap.insert(166, QS("PgBack"));
    keyMap.insert(167, QS("PgForward"));
#endif
}

void InputMap::initModMap() {
    modMap.clear();
    modMap.insert(keyNameCtrl(),  Qt::ControlModifier);
    modMap.insert(keyNameAlt(),   Qt::AltModifier);
    modMap.insert(keyNameShift(), Qt::ShiftModifier);
}

QString InputMap::keyNameCtrl() {
#ifdef __APPLE__
    return QS("⌘");
#else
    return QS("Ctrl");
#endif
}

QString InputMap::keyNameAlt() {
#ifdef __APPLE__
    return QS("⌥");
#else
    return QS("Alt");
#endif
}

QString InputMap::keyNameShift() {
#ifdef __APPLE__
    return QS("⇧");
#else
    return QS("Shift");
#endif
}

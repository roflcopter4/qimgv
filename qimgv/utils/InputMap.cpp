#include "InputMap.h"

#if 1
enum class InputMap::KeyCode {
    INVALID_KEY = 0,

    // clang whines that `Key_0` is too similar to `Key_O`,
    // and that `Key_1` is too similar to `Key_I`.
    // Too bad I don't care.
    Key_0,
    Key_1,
    Key_2,
    Key_3,
    Key_4,
    Key_5,
    Key_6,
    Key_7,
    Key_8,
    Key_9,

    Key_A,
    Key_B,
    Key_C,
    Key_D,
    Key_E,
    Key_F,
    Key_G,
    Key_H,
    Key_I,
    Key_J,
    Key_K,
    Key_L,
    Key_M,
    Key_N,
    Key_O,
    Key_P,
    Key_Q,
    Key_R,
    Key_S,
    Key_T,
    Key_U,
    Key_V,
    Key_W,
    Key_X,
    Key_Y,
    Key_Z,

    Key_Esc,
    Key_F1,
    Key_F2,
    Key_F3,
    Key_F4,
    Key_F5,
    Key_F6,
    Key_F7,
    Key_F8,
    Key_F9,
    Key_F10,
    Key_F11,
    Key_F12,

    Key_Apostrophe,
    Key_Asterix,
    Key_BackSlash,
    Key_Backspace,
    Key_CapsLock,
    Key_CloseBracket,
    Key_Comma,
    Key_Del,
    Key_End,
    Key_Enter,
    Key_Equals,
    Key_FwdSlash,
    Key_Home,
    Key_Ins,
    Key_Menu,
    Key_Minus,
    Key_NumLock,
    Key_OpenBracket,
    Key_Pause,
    Key_Period,
    Key_Plus,
    Key_Print,
    Key_ScrollLock,
    Key_Semicolon,
    Key_Space,
    Key_Tab,
    Key_Tilde,

    Key_Up,
    Key_Down,
    Key_Left,
    Key_Right,
    Key_PgUp,
    Key_PgDown,

    Key_WakeUp,
    Key_LessThan,
    Key_PgBack,
    Key_PgForward,
};

std::map<uint32_t, InputMap::NativeInputKeyInfo> const InputMap::keyMap = {
    {0, {0, KeyCode::INVALID_KEY, u""_s}},

#if defined Q_OS_WINDOWS

    // Row 1
    {1,   {1,   KeyCode::Key_Esc,        u"Esc"_s}},
    {59,  {59,  KeyCode::Key_F1,         u"F1"_s}},
    {60,  {60,  KeyCode::Key_F2,         u"F2"_s}},
    {61,  {61,  KeyCode::Key_F3,         u"F3"_s}},
    {62,  {62,  KeyCode::Key_F4,         u"F4"_s}},
    {63,  {63,  KeyCode::Key_F5,         u"F5"_s}},
    {64,  {64,  KeyCode::Key_F6,         u"F6"_s}},
    {65,  {65,  KeyCode::Key_F7,         u"F7"_s}},
    {66,  {66,  KeyCode::Key_F8,         u"F8"_s}},
    {67,  {67,  KeyCode::Key_F9,         u"F9"_s}},
    {68,  {68,  KeyCode::Key_F10,        u"F10"_s}},
    {87,  {87,  KeyCode::Key_F11,        u"F11"_s}},
    {88,  {88,  KeyCode::Key_F12,        u"F12"_s}},
    {70,  {70,  KeyCode::Key_ScrollLock, u"ScrollLock"_s}},
    {69,  {69,  KeyCode::Key_Pause,      u"Pause"_s}},
    //{??, {??, KeyCode::Key_Print,      u"Print"_s}},

    // Row 2
    {41,  {41,  KeyCode::Key_Tilde,     u"`"_s}},
    {2,   {2,   KeyCode::Key_1,         u"1"_s}},
    {3,   {3,   KeyCode::Key_2,         u"2"_s}},
    {4,   {4,   KeyCode::Key_3,         u"3"_s}},
    {5,   {5,   KeyCode::Key_4,         u"4"_s}},
    {6,   {6,   KeyCode::Key_5,         u"5"_s}},
    {7,   {7,   KeyCode::Key_6,         u"6"_s}},
    {8,   {8,   KeyCode::Key_7,         u"7"_s}},
    {9,   {9,   KeyCode::Key_8,         u"8"_s}},
    {10,  {10,  KeyCode::Key_9,         u"9"_s}},
    {11,  {11,  KeyCode::Key_0,         u"0"_s}},
    {12,  {12,  KeyCode::Key_Minus,     u"-"_s}},
    {13,  {13,  KeyCode::Key_Equals,    u"="_s}},
    {14,  {14,  KeyCode::Key_Backspace, u"Backspace"_s}},
    {338, {338, KeyCode::Key_Ins,       u"Ins"_s}},
    {327, {327, KeyCode::Key_Home,      u"Home"_s}},
    {329, {329, KeyCode::Key_PgUp,      u"PgUp"_s}},

    // Row 3
    {15,  {15,  KeyCode::Key_Tab,          u"Tab"_s}},
    {16,  {16,  KeyCode::Key_Q,            u"Q"_s}},
    {17,  {17,  KeyCode::Key_W,            u"W"_s}},
    {18,  {18,  KeyCode::Key_E,            u"E"_s}},
    {19,  {19,  KeyCode::Key_R,            u"R"_s}},
    {20,  {20,  KeyCode::Key_T,            u"T"_s}},
    {21,  {21,  KeyCode::Key_Y,            u"Y"_s}},
    {22,  {22,  KeyCode::Key_U,            u"U"_s}},
    {23,  {23,  KeyCode::Key_I,            u"I"_s}},
    {24,  {24,  KeyCode::Key_O,            u"O"_s}},
    {25,  {25,  KeyCode::Key_P,            u"P"_s}},
    {26,  {26,  KeyCode::Key_OpenBracket,  u"["_s}},
    {27,  {27,  KeyCode::Key_CloseBracket, u"]"_s}},
    {28,  {28,  KeyCode::Key_Enter,        u"Enter"_s}},
    {339, {339, KeyCode::Key_Del,          u"Del"_s}},
    {335, {335, KeyCode::Key_End,          u"End"_s}},
    {337, {337, KeyCode::Key_PgDown,       u"PgDown"_s}},

    // Row 4
    {58,  {58,  KeyCode::Key_CapsLock,   u"CapsLock"_s}},
    {30,  {30,  KeyCode::Key_A,          u"A"_s}},
    {31,  {31,  KeyCode::Key_S,          u"S"_s}},
    {32,  {32,  KeyCode::Key_D,          u"D"_s}},
    {33,  {33,  KeyCode::Key_F,          u"F"_s}},
    {34,  {34,  KeyCode::Key_G,          u"G"_s}},
    {35,  {35,  KeyCode::Key_H,          u"H"_s}},
    {36,  {36,  KeyCode::Key_J,          u"J"_s}},
    {37,  {37,  KeyCode::Key_K,          u"K"_s}},
    {38,  {38,  KeyCode::Key_L,          u"L"_s}},
    {39,  {39,  KeyCode::Key_Semicolon,  u";"_s}},
    {40,  {40,  KeyCode::Key_Apostrophe, u"'"_s}},
    {43,  {43,  KeyCode::Key_BackSlash,  u"\\"_s}},

    // Row 5
    {44,  {44,  KeyCode::Key_Z,        u"Z"_s}},
    {45,  {45,  KeyCode::Key_X,        u"X"_s}},
    {46,  {46,  KeyCode::Key_C,        u"C"_s}},
    {47,  {47,  KeyCode::Key_V,        u"V"_s}},
    {48,  {48,  KeyCode::Key_B,        u"B"_s}},
    {49,  {49,  KeyCode::Key_N,        u"N"_s}},
    {50,  {50,  KeyCode::Key_M,        u"M"_s}},
    {51,  {51,  KeyCode::Key_Comma,    u","_s}},
    {52,  {52,  KeyCode::Key_Period,   u"."_s}},
    {53,  {53,  KeyCode::Key_FwdSlash, u"/"_s}},
    {328, {328, KeyCode::Key_Up,       u"Up"_s}},

    // Row 6
    {57,  {57,  KeyCode::Key_Space, u"Space"_s}},
    {349, {349, KeyCode::Key_Menu,  u"Menu"_s}},
    {331, {331, KeyCode::Key_Left,  u"Left"_s}},
    {336, {336, KeyCode::Key_Down,  u"Down"_s}},
    {333, {333, KeyCode::Key_Right, u"Right"_s}},

    // Numpad
    {325, {325, KeyCode::Key_NumLock,  u"NumLock"_s}},
    {309, {309, KeyCode::Key_FwdSlash, u"/"_s}},
    {55,  {55,  KeyCode::Key_Asterix,  u"*"_s}},
    {74,  {74,  KeyCode::Key_Minus,    u"-"_s}},
    {71,  {71,  KeyCode::Key_7,        u"7"_s}},
    {72,  {72,  KeyCode::Key_8,        u"8"_s}},
    {73,  {73,  KeyCode::Key_9,        u"9"_s}},
    {78,  {78,  KeyCode::Key_Plus,     u"+"_s}},
    {75,  {75,  KeyCode::Key_4,        u"4"_s}},
    {76,  {76,  KeyCode::Key_5,        u"5"_s}},
    {77,  {77,  KeyCode::Key_6,        u"6"_s}},
    {79,  {79,  KeyCode::Key_1,        u"1"_s}},
    {80,  {80,  KeyCode::Key_2,        u"2"_s}},
    {81,  {81,  KeyCode::Key_3,        u"3"_s}},
    {284, {284, KeyCode::Key_Enter,    u"Enter"_s}},
    {82,  {82,  KeyCode::Key_0,        u"0"_s}},
    {83,  {83,  KeyCode::Key_Period,   u"."_s}},

    {57372, {57372, KeyCode::Key_Enter,    u"Enter"_s}},
    {57397, {57397, KeyCode::Key_FwdSlash, u"/"_s}},
    {57413, {57413, KeyCode::Key_NumLock,  u"NumLock"_s}},
    {57415, {57424, KeyCode::Key_Home,     u"Home"_s}},
    {57416, {57423, KeyCode::Key_Up,       u"Up"_s}},
    {57417, {57419, KeyCode::Key_PgUp,     u"PgUp"_s}},
    {57419, {57417, KeyCode::Key_Left,     u"Left"_s}},
    {57421, {57426, KeyCode::Key_Right,    u"Right"_s}},
    {57423, {57416, KeyCode::Key_End,      u"End"_s}},
    {57424, {57415, KeyCode::Key_Down,     u"Down"_s}},
    {57425, {57425, KeyCode::Key_PgDown,   u"PgDown"_s}},
    {57426, {57421, KeyCode::Key_Ins,      u"Ins"_s}},
    {57427, {57437, KeyCode::Key_Del,      u"Del"_s}},
    {57437, {57427, KeyCode::Key_Menu,     u"Menu"_s}},

#elif defined Q_OS_LINUX
   // Row 1
   {9,   {9,   KeyCode::Key_Esc,        u"Esc"_s}},
   {67,  {67,  KeyCode::Key_F1,         u"F1"_s}},
   {68,  {68,  KeyCode::Key_F2,         u"F2"_s}},
   {69,  {69,  KeyCode::Key_F3,         u"F3"_s}},
   {70,  {70,  KeyCode::Key_F4,         u"F4"_s}},
   {71,  {71,  KeyCode::Key_F5,         u"F5"_s}},
   {72,  {72,  KeyCode::Key_F6,         u"F6"_s}},
   {73,  {73,  KeyCode::Key_F7,         u"F7"_s}},
   {74,  {74,  KeyCode::Key_F8,         u"F8"_s}},
   {75,  {75,  KeyCode::Key_F9,         u"F9"_s}},
   {76,  {76,  KeyCode::Key_F10,        u"F10"_s}},
   {95,  {95,  KeyCode::Key_F11,        u"F11"_s}},
   {96,  {96,  KeyCode::Key_F12,        u"F12"_s}},
   {107, {107, KeyCode::Key_Print,      u"Print"_s}},
   {78,  {78,  KeyCode::Key_ScrollLock, u"ScrollLock"_s}},
   {127, {127, KeyCode::Key_Pause,      u"Pause"_s}},

   // Row 2
   {49,  {49,  KeyCode::Key_Tilde,     u"`"_s}},
   {10,  {10,  KeyCode::Key_1,         u"1"_s}},
   {11,  {11,  KeyCode::Key_2,         u"2"_s}},
   {12,  {12,  KeyCode::Key_3,         u"3"_s}},
   {13,  {13,  KeyCode::Key_4,         u"4"_s}},
   {14,  {14,  KeyCode::Key_5,         u"5"_s}},
   {15,  {15,  KeyCode::Key_6,         u"6"_s}},
   {16,  {16,  KeyCode::Key_7,         u"7"_s}},
   {17,  {17,  KeyCode::Key_8,         u"8"_s}},
   {18,  {18,  KeyCode::Key_9,         u"9"_s}},
   {19,  {19,  KeyCode::Key_0,         u"0"_s}},
   {20,  {20,  KeyCode::Key_Minus,     u"-"_s}},
   {21,  {21,  KeyCode::Key_Equals,    u"="_s}},
   {22,  {22,  KeyCode::Key_Backspace, u"Backspace"_s}},
   {118, {118, KeyCode::Key_Ins,       u"Ins"_s}},
   {110, {110, KeyCode::Key_Home,      u"Home"_s}},
   {112, {112, KeyCode::Key_PgUp,      u"PgUp"_s}},

   // Row 3
   {23,  {23,  KeyCode::Key_Tab,          u"Tab"_s}},
   {24,  {24,  KeyCode::Key_Q,            u"Q"_s}},
   {25,  {25,  KeyCode::Key_W,            u"W"_s}},
   {26,  {26,  KeyCode::Key_E,            u"E"_s}},
   {27,  {27,  KeyCode::Key_R,            u"R"_s}},
   {28,  {28,  KeyCode::Key_T,            u"T"_s}},
   {29,  {29,  KeyCode::Key_Y,            u"Y"_s}},
   {30,  {30,  KeyCode::Key_U,            u"U"_s}},
   {31,  {31,  KeyCode::Key_I,            u"I"_s}},
   {32,  {32,  KeyCode::Key_O,            u"O"_s}},
   {33,  {33,  KeyCode::Key_P,            u"P"_s}},
   {34,  {34,  KeyCode::Key_OpenBracket,  u"["_s}},
   {35,  {35,  KeyCode::Key_CloseBracket, u"]"_s}},
   {36,  {36,  KeyCode::Key_Enter,        u"Enter"_s}},
   {119, {119, KeyCode::Key_Del,          u"Del"_s}},
   {115, {115, KeyCode::Key_End,          u"End"_s}},
   {117, {117, KeyCode::Key_PgDown,       u"PgDown"_s}},

   // Row 4
   {66,  {66,  KeyCode::Key_CapsLock,   u"CapsLock"_s}},
   {38,  {38,  KeyCode::Key_A,          u"A"_s}},
   {39,  {39,  KeyCode::Key_S,          u"S"_s}},
   {40,  {40,  KeyCode::Key_D,          u"D"_s}},
   {41,  {41,  KeyCode::Key_F,          u"F"_s}},
   {42,  {42,  KeyCode::Key_G,          u"G"_s}},
   {43,  {43,  KeyCode::Key_H,          u"H"_s}},
   {44,  {44,  KeyCode::Key_J,          u"J"_s}},
   {45,  {45,  KeyCode::Key_K,          u"K"_s}},
   {46,  {46,  KeyCode::Key_L,          u"L"_s}},
   {47,  {47,  KeyCode::Key_Semicolon,  u";"_s}},
   {48,  {48,  KeyCode::Key_Apostrophe, u"'"_s}},
   {51,  {51,  KeyCode::Key_BackSlash,  u"\\"_s}},

   // Row 5
   {52,  {52,  KeyCode::Key_Z,        u"Z"_s}},
   {53,  {53,  KeyCode::Key_X,        u"X"_s}},
   {54,  {54,  KeyCode::Key_C,        u"C"_s}},
   {55,  {55,  KeyCode::Key_V,        u"V"_s}},
   {56,  {56,  KeyCode::Key_B,        u"B"_s}},
   {57,  {57,  KeyCode::Key_N,        u"N"_s}},
   {58,  {58,  KeyCode::Key_M,        u"M"_s}},
   {59,  {59,  KeyCode::Key_Comma,    u","_s}},
   {60,  {60,  KeyCode::Key_Period,   u"."_s}},
   {61,  {61,  KeyCode::Key_FwdSlash, u"/"_s}},
   {111, {111, KeyCode::Key_Up,       u"Up"_s}},

   // Row 6
   {57,  {57,  KeyCode::Key_Space, u"Space"_s}},
   {349, {349, KeyCode::Key_Menu,  u"Menu"_s}},
   {331, {331, KeyCode::Key_Left,  u"Left"_s}},
   {336, {336, KeyCode::Key_Down,  u"Down"_s}},
   {333, {333, KeyCode::Key_Right, u"Right"_s}},

   // Numpad
   {77,  {77,  KeyCode::Key_NumLock,  u"NumLock"_s}},
   {106, {106, KeyCode::Key_FwdSlash, u"/"_s}},
   {63,  {63,  KeyCode::Key_Asterix,  u"*"_s}},
   {82,  {82,  KeyCode::Key_Minus,    u"-"_s}},
   {79,  {79,  KeyCode::Key_7,        u"7"_s}},
   {80,  {80,  KeyCode::Key_8,        u"8"_s}},
   {81,  {81,  KeyCode::Key_9,        u"9"_s}},
   {86,  {86,  KeyCode::Key_Plus,     u"+"_s}},
   {83,  {83,  KeyCode::Key_4,        u"4"_s}},
   {84,  {84,  KeyCode::Key_5,        u"5"_s}},
   {85,  {85,  KeyCode::Key_6,        u"6"_s}},
   {87,  {87,  KeyCode::Key_1,        u"1"_s}},
   {88,  {88,  KeyCode::Key_2,        u"2"_s}},
   {89,  {89,  KeyCode::Key_3,        u"3"_s}},
   {104, {104, KeyCode::Key_Enter,    u"Enter"_s}},
   {90,  {90,  KeyCode::Key_0,        u"0"_s}},
   {91,  {91,  KeyCode::Key_Period,   u"."_s}},

   // Special
   {151, {151, KeyCode::Key_WakeUp,    u"WakeUp"_s}},
   {94,  {94,  KeyCode::Key_LessThan,  u"<"_s}},
   {166, {166, KeyCode::Key_PgBack,    u"PgBack"_s}},
   {167, {167, KeyCode::Key_PgForward, u"PgForward"_s}},

#endif
};
#endif

std::map<QString, Qt::KeyboardModifier> const &
InputMap::modifiers()
{
    return modMap;
}

bool InputMap::keyMapContains(uint32_t nativeScanCode)
{
    return keyMap.contains(nativeScanCode);
}

QString InputMap::keyName(uint32_t nativeScanCode)
{
    if (keyMap.contains(nativeScanCode))
        return keyMap.at(nativeScanCode).repr;
    return u""_s;
}

std::map<QString, Qt::KeyboardModifier> const InputMap::modMap = {
    {keyNameCtrl(),  Qt::ControlModifier},
    {keyNameAlt(),   Qt::AltModifier},
    {keyNameShift(), Qt::ShiftModifier},
};

QString InputMap::keyNameCtrl()
{
#ifdef __APPLE__
    return u"⌘"_s;
#else
    return u"Ctrl"_s;
#endif
}

QString InputMap::keyNameAlt()
{
#ifdef __APPLE__
    return u"⌥"_s;
#else
    return u"Alt"_s;
#endif
}

QString InputMap::keyNameShift()
{
#ifdef __APPLE__
    return u"⇧"_s;
#else
    return u"Shift"_s;
#endif
}

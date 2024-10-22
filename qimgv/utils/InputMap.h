#pragma once

#include <QMap>
#include <QString>

class InputMap
{
  public:
    InputMap() = delete;

    static QString keyNameCtrl();
    static QString keyNameAlt();
    static QString keyNameShift();

    ND static auto modifiers() -> std::map<QString, Qt::KeyboardModifier> const &;
    ND static bool keyMapContains(uint32_t nativeScanCode);
    ND static auto keyName(uint32_t nativeScanCode) -> QString;

  private:
    enum class KeyCode;
    struct NativeInputKeyInfo {
        uint32_t nativeCode;
        KeyCode  key;
        QString  repr;
    };

    static std::map<uint32_t, NativeInputKeyInfo> const keyMap;
    static std::map<QString, Qt::KeyboardModifier> const modMap;
};

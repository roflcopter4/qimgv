#include "ActionManager.h"

ActionManager *actionManager = nullptr;


ActionManager::ActionManager(QObject *parent)
    : QObject(parent)
{}

ActionManager::~ActionManager() = default;

ActionManager *ActionManager::getInstance()
{
    static std::mutex mtx;
    std::lock_guard   lock(mtx);

    if (!actionManager) {
        actionManager = new ActionManager();
        actionManager->initDefaults();
        actionManager->initShortcuts();
    }
    return actionManager;
}

//------------------------------------------------------------------------------

void ActionManager::initDefaults()
{
    defaults.insert(u"Right"_s,               u"nextImage"_s);
    defaults.insert(u"Left"_s,                u"prevImage"_s);
    defaults.insert(u"XButton2"_s,            u"nextImage"_s);
    defaults.insert(u"XButton1"_s,            u"prevImage"_s);
    defaults.insert(u"WheelDown"_s,           u"nextImage"_s);
    defaults.insert(u"WheelUp"_s,             u"prevImage"_s);
    defaults.insert(u"F"_s,                   u"toggleFullscreen"_s);
    defaults.insert(u"F11"_s,                 u"toggleFullscreen"_s);
    defaults.insert(u"LMB_DoubleClick"_s,     u"toggleFullscreen"_s);
    defaults.insert(u"MiddleButton"_s,        u"exit"_s);
    defaults.insert(u"Space"_s,               u"toggleFitMode"_s);
    defaults.insert(u"1"_s,                   u"fitWindow"_s);
    defaults.insert(u"2"_s,                   u"fitWidth"_s);
    defaults.insert(u"3"_s,                   u"fitNormal"_s);
    defaults.insert(u"R"_s,                   u"resize"_s);
    defaults.insert(u"H"_s,                   u"flipH"_s);
    defaults.insert(u"V"_s,                   u"flipV"_s);
    defaults.insert(InputMap::keyNameCtrl() + u"+R", u"rotateRight"_s);
    defaults.insert(InputMap::keyNameCtrl() + u"+L", u"rotateLeft"_s);
    defaults.insert(InputMap::keyNameCtrl() + u"+WheelUp", u"zoomInCursor"_s);
    defaults.insert(InputMap::keyNameCtrl() + u"+WheelDown", u"zoomOutCursor"_s);
    defaults.insert(u"+"_s,                   u"zoomIn"_s);
    defaults.insert(u"-"_s,                   u"zoomOut"_s);
    defaults.insert(InputMap::keyNameCtrl() + u"+Down", u"zoomOut"_s);
    defaults.insert(InputMap::keyNameCtrl() + u"+Up", u"zoomIn"_s);
    defaults.insert(u"Up"_s,                  u"scrollUp"_s);
    defaults.insert(u"Down"_s,                u"scrollDown"_s);
    defaults.insert(InputMap::keyNameCtrl() + u"+O", u"open"_s);
    defaults.insert(InputMap::keyNameCtrl() + u"+S", u"save"_s);
    defaults.insert(InputMap::keyNameCtrl() + u'+' + InputMap::keyNameShift() + u"+S", u"saveAs"_s);
    defaults.insert(InputMap::keyNameCtrl() + u"+W", u"setWallpaper"_s);
    defaults.insert(u"X"_s,                   u"crop"_s);
    defaults.insert(InputMap::keyNameCtrl() + u"+P", u"print"_s);
    defaults.insert(InputMap::keyNameAlt() + u"+X", u"exit"_s);
    defaults.insert(InputMap::keyNameCtrl() +  u"+Q", u"exit"_s);
    defaults.insert(u"Esc"_s,                 u"closeFullScreenOrExit"_s);
    defaults.insert(u"Del"_s,                 u"moveToTrash"_s);
    defaults.insert(InputMap::keyNameShift() + u"+Del", u"removeFile"_s);
    defaults.insert(u"C"_s,                   u"copyFile"_s);
    defaults.insert(u"M"_s,                   u"moveFile"_s);
    defaults.insert(u"Home"_s,                u"jumpToFirst"_s);
    defaults.insert(u"End"_s,                 u"jumpToLast"_s);
    defaults.insert(InputMap::keyNameCtrl() + u"+Right", u"seekVideoForward"_s);
    defaults.insert(InputMap::keyNameCtrl() + u"+Left", u"seekVideoBackward"_s);
    defaults.insert(u","_s,                   u"frameStepBack"_s);
    defaults.insert(u"."_s,                   u"frameStep"_s);
    defaults.insert(u"Enter"_s,               u"folderView"_s);
    defaults.insert(u"Backspace"_s,           u"folderView"_s);
    defaults.insert(u"F5"_s,                  u"reloadImage"_s);
    defaults.insert(InputMap::keyNameCtrl() + u"+C", u"copyFileClipboard"_s);
    defaults.insert(InputMap::keyNameCtrl() + u'+' + InputMap::keyNameShift() + u"+C", u"copyPathClipboard"_s);
    defaults.insert(u"F2"_s,                  u"renameFile"_s);
    defaults.insert(u"RMB"_s,                 u"contextMenu"_s);
    defaults.insert(u"Menu"_s,                u"contextMenu"_s);
    defaults.insert(u"I"_s,                   u"toggleImageInfo"_s);
    defaults.insert(InputMap::keyNameCtrl() + u"+`", u"toggleShuffle"_s);
    defaults.insert(InputMap::keyNameCtrl() + u"+D", u"showInDirectory"_s);
    defaults.insert(u"`"_s,                   u"toggleSlideshow"_s);
    defaults.insert(InputMap::keyNameCtrl() + u"+Z", u"discardEdits"_s);
    defaults.insert(InputMap::keyNameShift() + u"+Right", u"nextDirectory"_s);
    defaults.insert(InputMap::keyNameShift() + u"+Left", u"prevDirectory"_s);
    defaults.insert(InputMap::keyNameShift() + u"+F", u"toggleFullscreenInfoBar"_s);
    defaults.insert(InputMap::keyNameCtrl() + u"+V", u"pasteFile"_s);

#ifdef __APPLE__
    defaults.insert(InputMap::keyNameAlt() + u"+Up"_s, u"zoomIn"_s);
    defaults.insert(InputMap::keyNameAlt() + u"+Down"_s, u"zoomOut"_s);
    defaults.insert(InputMap::keyNameCtrl() + u"+Comma"_s, u"openSettings"_s);
#else
    defaults.insert(u"P"_s, u"openSettings"_s);
#endif

    // defaults.insert(u"Backspace"_s, u"goUp"_s); // todo: shortcut scopes?
}

void ActionManager::initShortcuts()
{
    readShortcuts();
    if (shortcuts.isEmpty())
        resetDefaults();
}

void ActionManager::addShortcut(QString const &keys, QString const &action)
{
    ActionType type = validateAction(action);
    if (type != ActionType::INVALID)
        shortcuts.insert(keys, action);
}

void ActionManager::removeShortcut(QString const &keys)
{
    shortcuts.remove(keys);
}

QStringList ActionManager::actionList()
{
    return appActions->getList();
}

QMap<QString, QString> const &ActionManager::allShortcuts()
{
    return shortcuts;
}

void ActionManager::removeAllShortcuts()
{
    shortcuts.clear();
}

// Removes all shortcuts for specified action. Slow (reverse map lookup).
void ActionManager::removeAllShortcuts(QString const &actionName)
{
    if (validateAction(actionName) == ActionType::INVALID)
        return;

    for (auto i = shortcuts.begin(); i != shortcuts.end();)
        if (i.value() == actionName)
            i = shortcuts.erase(i);
        else
            ++i;
}

QString ActionManager::keyForNativeScancode(quint32 scanCode)
{
    return InputMap::keyName(scanCode);
}

void ActionManager::resetDefaults()
{
    shortcuts = actionManager->defaults;
}

void ActionManager::resetDefaults(QString const &action)
{
    removeAllShortcuts(action);
    QMapIterator<QString, QString> i(defaults);
    while (i.hasNext()) {
        i.next();
        if (i.value() == action) {
            shortcuts.insert(i.key(), i.value());
            qDebug() << u"[ActionManager] new action " << i.value() << u" - assigning as [" << i.key() << u']';
        }
    }
}

void ActionManager::adjustFromVersion(QVersionNumber const &lastVer)
{
    // swap Ctrl-P & P
    if (lastVer < QVersionNumber(0, 9, 2)) {
        resetDefaults(u"print"_s);
        resetDefaults(u"openSettings"_s);
    }
    // swap WheelUp/WheelDown. derp
    if (lastVer < QVersionNumber(1, 0, 1)) {
        qDebug() << u"[actionManager]: swapping WheelUp/WheelDown";
        QMapIterator<QString, QString> i(shortcuts);
        QMap<QString, QString>         swapped;
        while (i.hasNext()) {
            i.next();
            QString key = i.key();
            if (key.contains(u"WheelUp"_sv))
                key.replace(u"WheelUp"_s, u"WheelDown"_s);
            else if (key.contains(u"WheelDown"_sv))
                key.replace(u"WheelDown"_s, u"WheelUp"_s);
            swapped.insert(key, i.value());
        }
        shortcuts = swapped;
    }
    // add new default actions
    QMapIterator<QString, QString> i(defaults);
    while (i.hasNext()) {
        i.next();
        if (appActions->getMap().value(i.value()) > lastVer) {
            if (!shortcuts.contains(i.key())) {
                shortcuts.insert(i.key(), i.value());
                qDebug() << u"[ActionManager] new action " << i.value() << u" - assigning as [" << i.key() << u']';
            } else if (i.value() != actionForShortcut(i.key())) {
                qDebug() << u"[ActionManager] new action " << i.value() << u" - shortcut [" << i.key()
                         << u"] already assigned to another action " << actionForShortcut(i.key());
            }
        }
    }
    // apply
    saveShortcuts();
}

void ActionManager::saveShortcuts()
{
    settings->saveShortcuts(shortcuts);
}

QString ActionManager::actionForShortcut(QString const &keys)
{
    return shortcuts[keys];
}

// returns first shortcut that is found
QString ActionManager::shortcutForAction(QString const &action) const
{
    return shortcuts.key(action, u""_s);
}

QStringList ActionManager::shortcutsForAction(QString const &action) const
{
    return shortcuts.keys(action);
}

bool ActionManager::invokeAction(QString const &actionName)
{
    ActionType type = validateAction(actionName);
    if (type == ActionType::NORMAL) {
        QMetaObject::invokeMethod(this, actionName.toLatin1().constData(), Qt::DirectConnection);
        return true;
    } else if (type == ActionType::SCRIPT) {
        QString scriptName = actionName;
        scriptName.remove(0, 2); // remove the u"s:"_s prefix
        emit runScript(scriptName);
        return true;
    }
    return false;
}

bool ActionManager::invokeActionForShortcut(QString const &shortcut)
{
    if (!shortcut.isEmpty() && shortcuts.contains(shortcut))
        return invokeAction(shortcuts[shortcut]);
    return false;
}

void ActionManager::validateShortcuts()
{
    for (auto i = shortcuts.begin(); i != shortcuts.end();)
        if (validateAction(i.value()) == ActionType::INVALID)
            i = shortcuts.erase(i);
        else
            ++i;
}

ActionType ActionManager::validateAction(QString const &actionName)
{
    if (appActions->getMap().contains(actionName))
        return ActionType::NORMAL;
    if (actionName.startsWith(u"s:"_sv)) {
        QString scriptName = actionName;
        scriptName.remove(0, 2);
        if (scriptManager->scriptExists(scriptName))
            return ActionType::SCRIPT;
    }
    return ActionType::INVALID;
}

void ActionManager::readShortcuts()
{
    settings->readShortcuts(shortcuts);
    validateShortcuts();
}

bool ActionManager::processEvent(QInputEvent *event)
{
    return invokeActionForShortcut(ShortcutBuilder::fromEvent(event));
}

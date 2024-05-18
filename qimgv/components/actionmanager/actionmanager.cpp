#include "actionmanager.h"

ActionManager *actionManager = nullptr;

ActionManager::ActionManager(QObject *parent)
    : QObject(parent)
{
}
//------------------------------------------------------------------------------
ActionManager::~ActionManager()
{
    delete actionManager;
}
//------------------------------------------------------------------------------
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
    defaults.insert("Right", "nextImage");
    defaults.insert("Left", "prevImage");
    defaults.insert("XButton2", "nextImage");
    defaults.insert("XButton1", "prevImage");
    defaults.insert("WheelDown", "nextImage");
    defaults.insert("WheelUp", "prevImage");
    defaults.insert("F", "toggleFullscreen");
    defaults.insert("F11", "toggleFullscreen");
    defaults.insert("LMB_DoubleClick", "toggleFullscreen");
    defaults.insert("MiddleButton", "exit");
    defaults.insert("Space", "toggleFitMode");
    defaults.insert("1", "fitWindow");
    defaults.insert("2", "fitWidth");
    defaults.insert("3", "fitNormal");
    defaults.insert("R", "resize");
    defaults.insert("H", "flipH");
    defaults.insert("V", "flipV");
    defaults.insert(InputMap::keyNameCtrl() + "+R", "rotateRight");
    defaults.insert(InputMap::keyNameCtrl() + "+L", "rotateLeft");
    defaults.insert(InputMap::keyNameCtrl() + "+WheelUp", "zoomInCursor");
    defaults.insert(InputMap::keyNameCtrl() + "+WheelDown", "zoomOutCursor");
    defaults.insert("+", "zoomIn");
    defaults.insert("-", "zoomOut");
    defaults.insert(InputMap::keyNameCtrl() + "+Down", "zoomOut");
    defaults.insert(InputMap::keyNameCtrl() + "+Up", "zoomIn");
    defaults.insert("Up", "scrollUp");
    defaults.insert("Down", "scrollDown");
    defaults.insert(InputMap::keyNameCtrl() + "+O", "open");
    defaults.insert(InputMap::keyNameCtrl() + "+S", "save");
    defaults.insert(InputMap::keyNameCtrl() + "+" + InputMap::keyNameShift() + "+S", "saveAs");
    defaults.insert(InputMap::keyNameCtrl() + "+W", "setWallpaper");
    defaults.insert("X", "crop");
    defaults.insert(InputMap::keyNameCtrl() + "+P", "print");
    defaults.insert(InputMap::keyNameAlt() + "+X", "exit");
    defaults.insert(InputMap::keyNameCtrl() + "+Q", "exit");
    defaults.insert("Esc", "closeFullScreenOrExit");
    defaults.insert("Del", "moveToTrash");
    defaults.insert(InputMap::keyNameShift() + "+Del", "removeFile");
    defaults.insert("C", "copyFile");
    defaults.insert("M", "moveFile");
    defaults.insert("Home", "jumpToFirst");
    defaults.insert("End", "jumpToLast");
    defaults.insert(InputMap::keyNameCtrl() + "+Right", "seekVideoForward");
    defaults.insert(InputMap::keyNameCtrl() + "+Left", "seekVideoBackward");
    defaults.insert(",", "frameStepBack");
    defaults.insert(".", "frameStep");
    defaults.insert("Enter", "folderView");
    defaults.insert("Backspace", "folderView");
    defaults.insert("F5", "reloadImage");
    defaults.insert(InputMap::keyNameCtrl() + "+C", "copyFileClipboard");
    defaults.insert(InputMap::keyNameCtrl() + "+" + InputMap::keyNameShift() + "+C", "copyPathClipboard");
    defaults.insert("F2", "renameFile");
    defaults.insert("RMB", "contextMenu");
    defaults.insert("Menu", "contextMenu");
    defaults.insert("I", "toggleImageInfo");
    defaults.insert(InputMap::keyNameCtrl() + "+`", "toggleShuffle");
    defaults.insert(InputMap::keyNameCtrl() + "+D", "showInDirectory");
    defaults.insert("`", "toggleSlideshow");
    defaults.insert(InputMap::keyNameCtrl() + "+Z", "discardEdits");
    defaults.insert(InputMap::keyNameShift() + "+Right", "nextDirectory");
    defaults.insert(InputMap::keyNameShift() + "+Left", "prevDirectory");
    defaults.insert(InputMap::keyNameShift() + "+F", "toggleFullscreenInfoBar");
    defaults.insert(InputMap::keyNameCtrl() + "+V", "pasteFile");

#ifdef __APPLE__
    defaults.insert(InputMap::keyNameAlt() + "+Up", "zoomIn");
    defaults.insert(InputMap::keyNameAlt() + "+Down", "zoomOut");
    defaults.insert(InputMap::keyNameCtrl() + "+Comma", "openSettings");
#else
    defaults.insert("P", "openSettings");
#endif

    // defaults.insert("Backspace", "goUp"); // todo: shortcut scopes?
}

//------------------------------------------------------------------------------
void ActionManager::initShortcuts()
{
    readShortcuts();
    if (shortcuts.isEmpty())
        resetDefaults();
}
//------------------------------------------------------------------------------
void ActionManager::addShortcut(QString const &keys, QString const &action)
{
    ActionType type = validateAction(action);
    if (type != ActionType::INVALID)
        shortcuts.insert(keys, action);
}
//------------------------------------------------------------------------------
void ActionManager::removeShortcut(QString const &keys)
{
    shortcuts.remove(keys);
}
//------------------------------------------------------------------------------
QStringList ActionManager::actionList()
{
    return appActions->getList();
}
//------------------------------------------------------------------------------
QMap<QString, QString> const &ActionManager::allShortcuts()
{
    return shortcuts;
}
//------------------------------------------------------------------------------
void ActionManager::removeAllShortcuts()
{
    shortcuts.clear();
}
//------------------------------------------------------------------------------
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
//------------------------------------------------------------------------------
QString ActionManager::keyForNativeScancode(quint32 scanCode)
{
    if (inputMap->keys().contains(scanCode))
        return inputMap->keys()[scanCode];
    return "";
}
//------------------------------------------------------------------------------
void ActionManager::resetDefaults()
{
    shortcuts = actionManager->defaults;
}
//------------------------------------------------------------------------------
void ActionManager::resetDefaults(QString const &action)
{
    removeAllShortcuts(action);
    QMapIterator<QString, QString> i(defaults);
    while (i.hasNext()) {
        i.next();
        if (i.value() == action) {
            shortcuts.insert(i.key(), i.value());
            qDebug() << "[ActionManager] new action " << i.value() << " - assigning as [" << i.key() << "]";
        }
    }
}
//------------------------------------------------------------------------------
void ActionManager::adjustFromVersion(QVersionNumber const &lastVer)
{
    // swap Ctrl-P & P
    if (lastVer < QVersionNumber(0, 9, 2)) {
        resetDefaults("print");
        resetDefaults("openSettings");
    }
    // swap WheelUp/WheelDown. derp
    if (lastVer < QVersionNumber(1, 0, 1)) {
        qDebug() << "[actionManager]: swapping WheelUp/WheelDown";
        QMapIterator<QString, QString> i(shortcuts);
        QMap<QString, QString>         swapped;
        while (i.hasNext()) {
            i.next();
            QString key = i.key();
            if (key.contains("WheelUp"))
                key.replace("WheelUp", "WheelDown");
            else if (key.contains("WheelDown"))
                key.replace("WheelDown", "WheelUp");
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
                qDebug() << "[ActionManager] new action " << i.value() << " - assigning as [" << i.key() << "]";
            } else if (i.value() != actionForShortcut(i.key())) {
                qDebug() << "[ActionManager] new action " << i.value() << " - shortcut [" << i.key()
                         << "] already assigned to another action " << actionForShortcut(i.key());
            }
        }
    }
    // apply
    saveShortcuts();
}
//------------------------------------------------------------------------------
void ActionManager::saveShortcuts()
{
    settings->saveShortcuts(shortcuts);
}
//------------------------------------------------------------------------------
QString ActionManager::actionForShortcut(QString const &keys)
{
    return shortcuts[keys];
}

// returns first shortcut that is found
QString ActionManager::shortcutForAction(QString const &action) const
{
    return shortcuts.key(action, "");
}

QStringList ActionManager::shortcutsForAction(QString const &action) const
{
    return shortcuts.keys(action);
}

//------------------------------------------------------------------------------
bool ActionManager::invokeAction(QString const &actionName)
{
    ActionType type = validateAction(actionName);
    if (type == ActionType::NORMAL) {
        QMetaObject::invokeMethod(this, actionName.toLatin1().constData(), Qt::DirectConnection);
        return true;
    } else if (type == ActionType::SCRIPT) {
        QString scriptName = actionName;
        scriptName.remove(0, 2); // remove the "s:" prefix
        emit runScript(scriptName);
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
bool ActionManager::invokeActionForShortcut(QString const &shortcut)
{
    if (!shortcut.isEmpty() && shortcuts.contains(shortcut))
        return invokeAction(shortcuts[shortcut]);
    return false;
}
//------------------------------------------------------------------------------
void ActionManager::validateShortcuts()
{
    for (auto i = shortcuts.begin(); i != shortcuts.end();)
        if (validateAction(i.value()) == ActionType::INVALID)
            i = shortcuts.erase(i);
        else
            ++i;
}
//------------------------------------------------------------------------------
ActionType ActionManager::validateAction(QString const &actionName)
{
    if (appActions->getMap().contains(actionName))
        return ActionType::NORMAL;
    if (actionName.startsWith("s:")) {
        QString scriptName = actionName;
        scriptName.remove(0, 2);
        if (scriptManager->scriptExists(scriptName))
            return ActionType::SCRIPT;
    }
    return ActionType::INVALID;
}
//------------------------------------------------------------------------------
void ActionManager::readShortcuts()
{
    settings->readShortcuts(shortcuts);
    validateShortcuts();
}
//------------------------------------------------------------------------------
bool ActionManager::processEvent(QInputEvent *event)
{
    return invokeActionForShortcut(ShortcutBuilder::fromEvent(event));
}
//------------------------------------------------------------------------------

#include "ActionManager.h"

ActionManager *actionManager = nullptr;


ActionManager::ActionManager(QObject *parent)
    : QObject(parent)
{
}

ActionManager::~ActionManager()
{
}

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
    defaults.insert(QS("Right"),               QS("nextImage"));
    defaults.insert(QS("Left"),                QS("prevImage"));
    defaults.insert(QS("XButton2"),            QS("nextImage"));
    defaults.insert(QS("XButton1"),            QS("prevImage"));
    defaults.insert(QS("WheelDown"),           QS("nextImage"));
    defaults.insert(QS("WheelUp"),             QS("prevImage"));
    defaults.insert(QS("F"),                   QS("toggleFullscreen"));
    defaults.insert(QS("F11"),                 QS("toggleFullscreen"));
    defaults.insert(QS("LMB_DoubleClick"),     QS("toggleFullscreen"));
    defaults.insert(QS("MiddleButton"),        QS("exit"));
    defaults.insert(QS("Space"),               QS("toggleFitMode"));
    defaults.insert(QS("1"),                   QS("fitWindow"));
    defaults.insert(QS("2"),                   QS("fitWidth"));
    defaults.insert(QS("3"),                   QS("fitNormal"));
    defaults.insert(QS("R"),                   QS("resize"));
    defaults.insert(QS("H"),                   QS("flipH"));
    defaults.insert(QS("V"),                   QS("flipV"));
    defaults.insert(InputMap::keyNameCtrl() + u"+R", QS("rotateRight"));
    defaults.insert(InputMap::keyNameCtrl() + u"+L", QS("rotateLeft"));
    defaults.insert(InputMap::keyNameCtrl() + u"+WheelUp", QS("zoomInCursor"));
    defaults.insert(InputMap::keyNameCtrl() + u"+WheelDown", QS("zoomOutCursor"));
    defaults.insert(QS("+"),                   QS("zoomIn"));
    defaults.insert(QS("-"),                   QS("zoomOut"));
    defaults.insert(InputMap::keyNameCtrl() + u"+Down", QS("zoomOut"));
    defaults.insert(InputMap::keyNameCtrl() + u"+Up", QS("zoomIn"));
    defaults.insert(QS("Up"),                  QS("scrollUp"));
    defaults.insert(QS("Down"),                QS("scrollDown"));
    defaults.insert(InputMap::keyNameCtrl() + u"+O", QS("open"));
    defaults.insert(InputMap::keyNameCtrl() + u"+S", QS("save"));
    defaults.insert(InputMap::keyNameCtrl() + u'+' + InputMap::keyNameShift() + u"+S", QS("saveAs"));
    defaults.insert(InputMap::keyNameCtrl() + u"+W", QS("setWallpaper"));
    defaults.insert(QS("X"),                   QS("crop"));
    defaults.insert(InputMap::keyNameCtrl() + u"+P", QS("print"));
    defaults.insert(InputMap::keyNameAlt() + u"+X", QS("exit"));
    defaults.insert(InputMap::keyNameCtrl() +  u"+Q", QS("exit"));
    defaults.insert(QS("Esc"),                 QS("closeFullScreenOrExit"));
    defaults.insert(QS("Del"),                 QS("moveToTrash"));
    defaults.insert(InputMap::keyNameShift() + u"+Del", QS("removeFile"));
    defaults.insert(QS("C"),                   QS("copyFile"));
    defaults.insert(QS("M"),                   QS("moveFile"));
    defaults.insert(QS("Home"),                QS("jumpToFirst"));
    defaults.insert(QS("End"),                 QS("jumpToLast"));
    defaults.insert(InputMap::keyNameCtrl() + u"+Right", QS("seekVideoForward"));
    defaults.insert(InputMap::keyNameCtrl() + u"+Left", QS("seekVideoBackward"));
    defaults.insert(QS(","),                   QS("frameStepBack"));
    defaults.insert(QS("."),                   QS("frameStep"));
    defaults.insert(QS("Enter"),               QS("folderView"));
    defaults.insert(QS("Backspace"),           QS("folderView"));
    defaults.insert(QS("F5"),                  QS("reloadImage"));
    defaults.insert(InputMap::keyNameCtrl() + u"+C", QS("copyFileClipboard"));
    defaults.insert(InputMap::keyNameCtrl() + u'+' + InputMap::keyNameShift() + u"+C", QS("copyPathClipboard"));
    defaults.insert(QS("F2"),                  QS("renameFile"));
    defaults.insert(QS("RMB"),                 QS("contextMenu"));
    defaults.insert(QS("Menu"),                QS("contextMenu"));
    defaults.insert(QS("I"),                   QS("toggleImageInfo"));
    defaults.insert(InputMap::keyNameCtrl() + u"+`", QS("toggleShuffle"));
    defaults.insert(InputMap::keyNameCtrl() + u"+D", QS("showInDirectory"));
    defaults.insert(QS("`"),                   QS("toggleSlideshow"));
    defaults.insert(InputMap::keyNameCtrl() + u"+Z", QS("discardEdits"));
    defaults.insert(InputMap::keyNameShift() + u"+Right", QS("nextDirectory"));
    defaults.insert(InputMap::keyNameShift() + u"+Left", QS("prevDirectory"));
    defaults.insert(InputMap::keyNameShift() + u"+F", QS("toggleFullscreenInfoBar"));
    defaults.insert(InputMap::keyNameCtrl() + u"+V", QS("pasteFile"));

#ifdef __APPLE__
    defaults.insert(InputMap::keyNameAlt() + QS("+Up"), QS("zoomIn"));
    defaults.insert(InputMap::keyNameAlt() + QS("+Down"), QS("zoomOut"));
    defaults.insert(InputMap::keyNameCtrl() + QS("+Comma"), QS("openSettings"));
#else
    defaults.insert(QS("P"), QS("openSettings"));
#endif

    // defaults.insert(QS("Backspace"), QS("goUp")); // todo: shortcut scopes?
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
        resetDefaults(QS("print"));
        resetDefaults(QS("openSettings"));
    }
    // swap WheelUp/WheelDown. derp
    if (lastVer < QVersionNumber(1, 0, 1)) {
        qDebug() << "[actionManager]: swapping WheelUp/WheelDown";
        QMapIterator<QString, QString> i(shortcuts);
        QMap<QString, QString>         swapped;
        while (i.hasNext()) {
            i.next();
            QString key = i.key();
            if (key.contains(QSV("WheelUp")))
                key.replace(QS("WheelUp"), QS("WheelDown"));
            else if (key.contains(QSV("WheelDown")))
                key.replace(QS("WheelDown"), QS("WheelUp"));
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
    return shortcuts.key(action, QS(""));
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
        scriptName.remove(0, 2); // remove the QS("s:") prefix
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
    if (actionName.startsWith(QSV("s:"))) {
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

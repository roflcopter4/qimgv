﻿#pragma once

#include "components/scriptManager/ScriptManager.h"
#include "Settings.h"
#include "ShortcutBuilder.h"
#include "utils/Actions.h"
#include <QDebug>
#include <QKeyEvent>
#include <QMap>
#include <QMouseEvent>
#include <QObject>
#include <QStringList>
#include <QWheelEvent>

enum class ActionType {
    INVALID,
    NORMAL,
    SCRIPT,
};

class ActionManager final : public QObject
{
    Q_OBJECT

  public:
    static ActionManager *getInstance();
    ~ActionManager() override;
    DELETE_COPY_MOVE_ROUTINES(ActionManager);

    static auto actionList() -> QStringList;
    static auto keyForNativeScancode(quint32 scanCode) -> QString;

    auto actionForShortcut(QString const &keys) -> QString;
    auto allShortcuts() -> QMap<QString, QString> const &;
    auto shortcutForAction(QString const &action) const -> QString;
    auto shortcutsForAction(QString const &action) const -> QStringList;
    bool processEvent(QInputEvent *);
    void addShortcut(QString const &keys, QString const &action);
    void adjustFromVersion(QVersionNumber const &lastVer);
    void removeAllShortcuts();
    void removeAllShortcuts(QString const &actionName);
    void removeShortcut(QString const &keys);
    void resetDefaults();
    void resetDefaults(QString const &action);
    void saveShortcuts();

  public Q_SLOTS:
    bool invokeAction(QString const &actionName);

  private:
    explicit ActionManager(QObject *parent = nullptr);

    QMap<QString, QString> defaults;
    QMap<QString, QString> shortcuts; // <shortcut, action>

    void initDefaults();
    void connectActions();
    void initShortcuts();

    QString modifierKeys(QEvent *event);
    bool    invokeActionForShortcut(QString const &shortcut);
    void    validateShortcuts();
    void    readShortcuts();

    static ActionType validateAction(QString const &actionName);

  Q_SIGNALS:
    void open();
    void save();
    void saveAs();
    void openSettings();
    void crop();
    void setWallpaper();
    void nextImage();
    void prevImage();
    void fitWindow();
    void fitWidth();
    void fitNormal();
    void flipH();
    void flipV();
    void toggleFitMode();
    void toggleFullscreen();
    void scrollUp();
    void scrollDown();
    void scrollLeft();
    void scrollRight();
    void zoomIn();
    void zoomOut();
    void zoomInCursor();
    void zoomOutCursor();
    void resize();
    void rotateLeft();
    void rotateRight();
    void exit();
    void removeFile();
    void copyFile();
    void moveFile();
    void closeFullScreenOrExit();
    void jumpToFirst();
    void jumpToLast();
    void folderView();
    void documentView();
    void runScript(QString const &);
    void pauseVideo();
    void seekVideoForward();
    void seekVideoBackward();
    void frameStep();
    void frameStepBack();
    void toggleFolderView();
    void moveToTrash();
    void reloadImage();
    void copyFileClipboard();
    void copyPathClipboard();
    void renameFile();
    void contextMenu();
    void toggleTransparencyGrid();
    void sortByName();
    void sortByTime();
    void sortBySize();
    void toggleImageInfo();
    void toggleShuffle();
    void toggleScalingFilter();
    void showInDirectory();
    void toggleMute();
    void volumeUp();
    void volumeDown();
    void toggleSlideshow();
    void discardEdits();
    void goUp();
    void nextDirectory();
    void prevDirectory();
    void lockZoom();
    void lockView();
    void print();
    void toggleFullscreenInfoBar();
    void pasteFile();
};

extern ActionManager *actionManager;

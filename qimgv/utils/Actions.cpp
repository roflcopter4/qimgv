#include "Actions.h"

Actions *appActions = nullptr;

Actions::Actions()
{
    init();
}

Actions *Actions::getInstance()
{
    if (!appActions)
        appActions = new Actions();
    return appActions;
}

QMap<QString, QVersionNumber> const &Actions::getMap() const
{
    return mActions;
}

QList<QString> Actions::getList() const
{
    return mActions.keys();
}

void Actions::init()
{
    mActions.insert(u"nextImage"_s,               QVersionNumber(0, 6,  2));
    mActions.insert(u"prevImage"_s,               QVersionNumber(0, 6,  2));
    mActions.insert(u"toggleFullscreen"_s,        QVersionNumber(0, 6,  2));
    mActions.insert(u"fitWindow"_s,               QVersionNumber(0, 6,  2));
    mActions.insert(u"fitWidth"_s,                QVersionNumber(0, 6,  2));
    mActions.insert(u"fitNormal"_s,               QVersionNumber(0, 6,  2));
    mActions.insert(u"toggleFitMode"_s,           QVersionNumber(0, 6,  2));
    mActions.insert(u"resize"_s,                  QVersionNumber(0, 6,  2));
    mActions.insert(u"rotateRight"_s,             QVersionNumber(0, 6,  2));
    mActions.insert(u"rotateLeft"_s,              QVersionNumber(0, 6,  2));
    mActions.insert(u"scrollUp"_s,                QVersionNumber(0, 6,  2));
    mActions.insert(u"scrollDown"_s,              QVersionNumber(0, 6,  2));
    mActions.insert(u"scrollLeft"_s,              QVersionNumber(0, 6,  2));
    mActions.insert(u"scrollRight"_s,             QVersionNumber(0, 6,  2));
    mActions.insert(u"zoomIn"_s,                  QVersionNumber(0, 6,  2));
    mActions.insert(u"zoomOut"_s,                 QVersionNumber(0, 6,  2));
    mActions.insert(u"zoomInCursor"_s,            QVersionNumber(0, 6,  2));
    mActions.insert(u"zoomOutCursor"_s,           QVersionNumber(0, 6,  2));
    mActions.insert(u"open"_s,                    QVersionNumber(0, 6,  2));
    mActions.insert(u"save"_s,                    QVersionNumber(0, 6,  2));
    mActions.insert(u"saveAs"_s,                  QVersionNumber(0, 6,  2));
    mActions.insert(u"setWallpaper"_s,            QVersionNumber(0, 9,  3));
    mActions.insert(u"crop"_s,                    QVersionNumber(0, 6,  2));
    mActions.insert(u"removeFile"_s,              QVersionNumber(0, 6,  2));
    mActions.insert(u"copyFile"_s,                QVersionNumber(0, 6,  2));
    mActions.insert(u"moveFile"_s,                QVersionNumber(0, 6,  2));
    mActions.insert(u"jumpToFirst"_s,             QVersionNumber(0, 6,  2));
    mActions.insert(u"jumpToLast"_s,              QVersionNumber(0, 6,  2));
    mActions.insert(u"openSettings"_s,            QVersionNumber(0, 6,  2));
    mActions.insert(u"closeFullScreenOrExit"_s,   QVersionNumber(0, 6,  2));
    mActions.insert(u"exit"_s,                    QVersionNumber(0, 6,  2));
    mActions.insert(u"flipH"_s,                   QVersionNumber(0, 6,  3));
    mActions.insert(u"flipV"_s,                   QVersionNumber(0, 6,  3));
    mActions.insert(u"folderView"_s,              QVersionNumber(0, 6, 85));
    mActions.insert(u"pauseVideo"_s,              QVersionNumber(0, 6, 85));
    mActions.insert(u"frameStep"_s,               QVersionNumber(0, 6, 85));
    mActions.insert(u"frameStepBack"_s,           QVersionNumber(0, 6, 85));
    mActions.insert(u"folderView"_s,              QVersionNumber(0, 6, 85));
    mActions.insert(u"documentView"_s,            QVersionNumber(0, 6, 88));
    mActions.insert(u"toggleFolderView"_s,        QVersionNumber(0, 6, 88));
    mActions.insert(u"moveToTrash"_s,             QVersionNumber(0, 6, 89));
    mActions.insert(u"reloadImage"_s,             QVersionNumber(0, 7, 80));
    mActions.insert(u"copyFileClipboard"_s,       QVersionNumber(0, 7, 80));
    mActions.insert(u"copyPathClipboard"_s,       QVersionNumber(0, 7, 80));
    mActions.insert(u"renameFile"_s,              QVersionNumber(0, 7, 80));
    mActions.insert(u"contextMenu"_s,             QVersionNumber(0, 7, 81));
    mActions.insert(u"toggleTransparencyGrid"_s,  QVersionNumber(0, 7, 82));
    mActions.insert(u"sortByName"_s,              QVersionNumber(0, 7, 83));
    mActions.insert(u"sortByTime"_s,              QVersionNumber(0, 7, 83));
    mActions.insert(u"sortBySize"_s,              QVersionNumber(0, 7, 83));
    mActions.insert(u"toggleImageInfo"_s,         QVersionNumber(0, 7, 84));
    mActions.insert(u"toggleShuffle"_s,           QVersionNumber(0, 8,  3));
    mActions.insert(u"toggleScalingFilter"_s,     QVersionNumber(0, 8,  3));
    mActions.insert(u"toggleMute"_s,              QVersionNumber(0, 8,  7));
    mActions.insert(u"volumeUp"_s,                QVersionNumber(0, 8,  7));
    mActions.insert(u"volumeDown"_s,              QVersionNumber(0, 8,  7));
    mActions.insert(u"toggleSlideshow"_s,         QVersionNumber(0, 8, 81));
    mActions.insert(u"showInDirectory"_s,         QVersionNumber(0, 8, 82));
    mActions.insert(u"goUp"_s,                    QVersionNumber(0, 9,  2));
    mActions.insert(u"discardEdits"_s,            QVersionNumber(0, 9,  2));
    mActions.insert(u"nextDirectory"_s,           QVersionNumber(0, 9,  2));
    mActions.insert(u"prevDirectory"_s,           QVersionNumber(0, 9,  2));
    mActions.insert(u"seekVideoForward"_s,        QVersionNumber(0, 9,  2));
    mActions.insert(u"seekVideoBackward"_s,       QVersionNumber(0, 9,  2));
    mActions.insert(u"lockZoom"_s,                QVersionNumber(0, 9,  2));
    mActions.insert(u"lockView"_s,                QVersionNumber(0, 9,  2));
    mActions.insert(u"print"_s,                   QVersionNumber(1, 0,  0));
    mActions.insert(u"toggleFullscreenInfoBar"_s, QVersionNumber(1, 0,  0));
    mActions.insert(u"pasteFile"_s,               QVersionNumber(1, 0,  3));
}

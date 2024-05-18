#include "actions.h"

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

QMap<QString, QVersionNumber> const &Actions::getMap() const &
{
    return mActions;
}

QList<QString> Actions::getList() const
{
    return mActions.keys();
}

void Actions::init()
{
    mActions.insert(QS("nextImage"),               QVersionNumber(0, 6,  2));
    mActions.insert(QS("prevImage"),               QVersionNumber(0, 6,  2));
    mActions.insert(QS("toggleFullscreen"),        QVersionNumber(0, 6,  2));
    mActions.insert(QS("fitWindow"),               QVersionNumber(0, 6,  2));
    mActions.insert(QS("fitWidth"),                QVersionNumber(0, 6,  2));
    mActions.insert(QS("fitNormal"),               QVersionNumber(0, 6,  2));
    mActions.insert(QS("toggleFitMode"),           QVersionNumber(0, 6,  2));
    mActions.insert(QS("resize"),                  QVersionNumber(0, 6,  2));
    mActions.insert(QS("rotateRight"),             QVersionNumber(0, 6,  2));
    mActions.insert(QS("rotateLeft"),              QVersionNumber(0, 6,  2));
    mActions.insert(QS("scrollUp"),                QVersionNumber(0, 6,  2));
    mActions.insert(QS("scrollDown"),              QVersionNumber(0, 6,  2));
    mActions.insert(QS("scrollLeft"),              QVersionNumber(0, 6,  2));
    mActions.insert(QS("scrollRight"),             QVersionNumber(0, 6,  2));
    mActions.insert(QS("zoomIn"),                  QVersionNumber(0, 6,  2));
    mActions.insert(QS("zoomOut"),                 QVersionNumber(0, 6,  2));
    mActions.insert(QS("zoomInCursor"),            QVersionNumber(0, 6,  2));
    mActions.insert(QS("zoomOutCursor"),           QVersionNumber(0, 6,  2));
    mActions.insert(QS("open"),                    QVersionNumber(0, 6,  2));
    mActions.insert(QS("save"),                    QVersionNumber(0, 6,  2));
    mActions.insert(QS("saveAs"),                  QVersionNumber(0, 6,  2));
    mActions.insert(QS("setWallpaper"),            QVersionNumber(0, 9,  3));
    mActions.insert(QS("crop"),                    QVersionNumber(0, 6,  2));
    mActions.insert(QS("removeFile"),              QVersionNumber(0, 6,  2));
    mActions.insert(QS("copyFile"),                QVersionNumber(0, 6,  2));
    mActions.insert(QS("moveFile"),                QVersionNumber(0, 6,  2));
    mActions.insert(QS("jumpToFirst"),             QVersionNumber(0, 6,  2));
    mActions.insert(QS("jumpToLast"),              QVersionNumber(0, 6,  2));
    mActions.insert(QS("openSettings"),            QVersionNumber(0, 6,  2));
    mActions.insert(QS("closeFullScreenOrExit"),   QVersionNumber(0, 6,  2));
    mActions.insert(QS("exit"),                    QVersionNumber(0, 6,  2));
    mActions.insert(QS("flipH"),                   QVersionNumber(0, 6,  3));
    mActions.insert(QS("flipV"),                   QVersionNumber(0, 6,  3));
    mActions.insert(QS("folderView"),              QVersionNumber(0, 6, 85));
    mActions.insert(QS("pauseVideo"),              QVersionNumber(0, 6, 85));
    mActions.insert(QS("frameStep"),               QVersionNumber(0, 6, 85));
    mActions.insert(QS("frameStepBack"),           QVersionNumber(0, 6, 85));
    mActions.insert(QS("folderView"),              QVersionNumber(0, 6, 85));
    mActions.insert(QS("documentView"),            QVersionNumber(0, 6, 88));
    mActions.insert(QS("toggleFolderView"),        QVersionNumber(0, 6, 88));
    mActions.insert(QS("moveToTrash"),             QVersionNumber(0, 6, 89));
    mActions.insert(QS("reloadImage"),             QVersionNumber(0, 7, 80));
    mActions.insert(QS("copyFileClipboard"),       QVersionNumber(0, 7, 80));
    mActions.insert(QS("copyPathClipboard"),       QVersionNumber(0, 7, 80));
    mActions.insert(QS("renameFile"),              QVersionNumber(0, 7, 80));
    mActions.insert(QS("contextMenu"),             QVersionNumber(0, 7, 81));
    mActions.insert(QS("toggleTransparencyGrid"),  QVersionNumber(0, 7, 82));
    mActions.insert(QS("sortByName"),              QVersionNumber(0, 7, 83));
    mActions.insert(QS("sortByTime"),              QVersionNumber(0, 7, 83));
    mActions.insert(QS("sortBySize"),              QVersionNumber(0, 7, 83));
    mActions.insert(QS("toggleImageInfo"),         QVersionNumber(0, 7, 84));
    mActions.insert(QS("toggleShuffle"),           QVersionNumber(0, 8,  3));
    mActions.insert(QS("toggleScalingFilter"),     QVersionNumber(0, 8,  3));
    mActions.insert(QS("toggleMute"),              QVersionNumber(0, 8,  7));
    mActions.insert(QS("volumeUp"),                QVersionNumber(0, 8,  7));
    mActions.insert(QS("volumeDown"),              QVersionNumber(0, 8,  7));
    mActions.insert(QS("toggleSlideshow"),         QVersionNumber(0, 8, 81));
    mActions.insert(QS("showInDirectory"),         QVersionNumber(0, 8, 82));
    mActions.insert(QS("goUp"),                    QVersionNumber(0, 9,  2));
    mActions.insert(QS("discardEdits"),            QVersionNumber(0, 9,  2));
    mActions.insert(QS("nextDirectory"),           QVersionNumber(0, 9,  2));
    mActions.insert(QS("prevDirectory"),           QVersionNumber(0, 9,  2));
    mActions.insert(QS("seekVideoForward"),        QVersionNumber(0, 9,  2));
    mActions.insert(QS("seekVideoBackward"),       QVersionNumber(0, 9,  2));
    mActions.insert(QS("lockZoom"),                QVersionNumber(0, 9,  2));
    mActions.insert(QS("lockView"),                QVersionNumber(0, 9,  2));
    mActions.insert(QS("print"),                   QVersionNumber(1, 0,  0));
    mActions.insert(QS("toggleFullscreenInfoBar"), QVersionNumber(1, 0,  0));
    mActions.insert(QS("pasteFile"),               QVersionNumber(1, 0,  3));
}

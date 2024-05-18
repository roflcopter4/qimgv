#pragma once

#include <QObject>
#include <memory>
#include "gui/idirectoryview.h"
#include "components/thumbnailer/thumbnailer.h"
#include "directorymodel.h"
#include "sharedresources.h"
#include <QMimeData>

//tmp
#include <QtSvg/QSvgRenderer>

class DirectoryPresenter : public QObject {
    Q_OBJECT
public:
    explicit DirectoryPresenter(QObject *parent = nullptr);

    void setView(std::shared_ptr<IDirectoryView> const &);
    void setModel(std::shared_ptr<DirectoryModel> const &newModel);
    void unsetModel();

    void selectAndFocus(int index) const;
    void selectAndFocus(QString const &path) const;

    void onFileRemoved(QString const &filePath, int index) const;
    void onFileRenamed(QString const &fromPath, int indexFrom, QString const &toPath, int indexTo) const;
    void onFileAdded(QString const &filePath) const;
    void onFileModified(QString const &filePath) const;

    void onDirRemoved(QString const &dirPath, int index) const;
    void onDirRenamed(QString const &fromPath, int indexFrom, QString const &toPath, int indexTo) const;
    void onDirAdded(QString const &dirPath) const;

    bool showDirs() const;
    void setShowDirs(bool mode);

    QList<QString> selectedPaths() const;


signals:
    void dirActivated(QString dirPath);
    void fileActivated(QString filePath);
    void draggedOut(QList<QString>);
    void droppedInto(QList<QString>, QString);

public slots:
    void disconnectView();
    void reloadModel();

private slots:
    void generateThumbnails(QList<int> const &, int, bool, bool);
    void onThumbnailReady(std::shared_ptr<Thumbnail> const &thumb, QString const &filePath) const;
    void populateView();
    void onItemActivated(int absoluteIndex);
    void onDraggedOut();
    void onDraggedOver(int index) const;

    void onDroppedInto(QMimeData const *data, QObject *source, int targetIndex);
private:
    std::shared_ptr<IDirectoryView> view = nullptr;
    std::shared_ptr<DirectoryModel> model = nullptr;
    Thumbnailer thumbnailer;
    bool mShowDirs;
};

#pragma once

#include "components/thumbnailer/thumbnailer.h"
#include "directorymodel.h"
#include "gui/idirectoryview.h"
#include "sharedresources.h"
#include <QMimeData>
#include <QObject>
#include <memory>

//tmp
#include <QtSvg/QSvgRenderer>

class DirectoryPresenter : public QObject
{
    Q_OBJECT

  public:
    explicit DirectoryPresenter(QObject *parent = nullptr);

    void setView(std::shared_ptr<IDirectoryView> newView);
    void setModel(std::shared_ptr<DirectoryModel> newModel);
    void unsetModel();

    void selectAndFocus(qsizetype index) const;
    void selectAndFocus(QString const &path) const;

    void onFileRemoved(QString const &filePath, qsizetype index) const;
    void onFileRenamed(QString const &fromPath, qsizetype indexFrom, QString const &toPath, qsizetype indexTo) const;
    void onFileAdded(QString const &filePath) const;
    void onFileModified(QString const &filePath) const;

    void onDirRemoved(QString const &dirPath, qsizetype index) const;
    void onDirRenamed(QString const &fromPath, qsizetype indexFrom, QString const &toPath, qsizetype indexTo) const;
    void onDirAdded(QString const &dirPath) const;

    bool showDirs() const;
    void setShowDirs(bool mode);

    QList<QString> selectedPaths() const;


  Q_SIGNALS:
    void dirActivated(QString dirPath);
    void fileActivated(QString filePath);
    void draggedOut(QList<QString>);
    void droppedInto(QList<QString>, QString);

  public Q_SLOTS:
    void disconnectView();
    void reloadModel();

  private Q_SLOTS:
    void generateThumbnails(IDirectoryView::SelectionList const &, int, bool, bool);
    void onThumbnailReady(std::shared_ptr<Thumbnail> const &thumb, QString const &filePath) const;
    void populateView();
    void onItemActivated(qsizetype absoluteIndex);
    void onDraggedOut();
    void onDraggedOver(qsizetype index) const;

    void onDroppedInto(QMimeData const *data, QObject *source, qsizetype targetIndex);

  private:
    std::shared_ptr<IDirectoryView> view  = nullptr;
    std::shared_ptr<DirectoryModel> model = nullptr;
    Thumbnailer                     thumbnailer;
    bool                            mShowDirs;
};

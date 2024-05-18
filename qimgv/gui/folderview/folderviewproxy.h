#pragma once

#include "gui/folderview/folderview.h"
#include <QMutexLocker>

struct FolderViewStateBuffer {
    QString      directory;
    QList<int>   selection;
    qsizetype    itemCount = 0;
    SortingMode  sortingMode;
    bool         fullscreenMode;
};

class FolderViewProxy : public QWidget, public IDirectoryView
{
    Q_OBJECT
    Q_INTERFACES(IDirectoryView)
  public:
    explicit FolderViewProxy(QWidget *parent = nullptr);

    void init();

  public slots:
    void populate(int) override;
    void setThumbnail(int pos, std::shared_ptr<Thumbnail> thumb) override;
    void select(QList<int>) override;
    void select(int) override;
    void focusOn(int) override;
    void focusOnSelection() override;
    void setDirectoryPath(QString path) override;
    void insertItem(int index) override;
    void removeItem(int index) override;
    void reloadItem(int index) override;
    void setDragHover(int) override;
    void addItem();
    void onFullscreenModeChanged(bool mode);
    void onSortingChanged(SortingMode mode);

    QList<int> &      selection() override;
    QList<int> const &selection() const override;

  protected:
    void showEvent(QShowEvent *event) override;

  signals:
    void itemActivated(int) override;
    void thumbnailsRequested(QList<int>, int, bool, bool) override;
    void draggedOut() override;
    void draggedToBookmarks(QList<int>) override;
    void sortingSelected(SortingMode);
    void showFoldersChanged(bool mode);
    void directorySelected(QString);
    void copyUrlsRequested(QList<QString>, QString path);
    void moveUrlsRequested(QList<QString>, QString path);
    void droppedInto(const QMimeData *, QObject *, int) override;
    void draggedOver(int) override;

  private:
    std::shared_ptr<FolderView> folderView;
    QVBoxLayout                 layout;
    FolderViewStateBuffer       stateBuf;
    QMutex                      m;
};

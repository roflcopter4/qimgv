#pragma once

#include "gui/folderView/FolderView.h"
#include <QMutexLocker>

class FolderViewProxy : public QWidget, public IDirectoryView
{
    Q_OBJECT
    Q_INTERFACES(IDirectoryView)

    struct StateBuffer {
        QString       directory;
        SelectionList selection;
        qsizetype     itemCount      = 0;
        SortingMode   sortingMode    = SortingMode::NAME;
        bool          fullscreenMode = false;
    };

  public:
    explicit FolderViewProxy(QWidget *parent);

    void init();

  public Q_SLOTS:
    void populate(qsizetype) override;
    void setThumbnail(qsizetype pos, QSharedPointer<Thumbnail> thumb) override;
    void select(SelectionList) override;
    void select(qsizetype) override;
    void focusOn(qsizetype) override;
    void focusOnSelection() override;
    void setDirectoryPath(QString path) override;
    void insertItem(qsizetype index) override;
    void removeItem(qsizetype index) override;
    void reloadItem(qsizetype index) override;
    void setDragHover(qsizetype) override;
    void addItem();
    void onFullscreenModeChanged(bool mode);
    void onSortingChanged(SortingMode mode);

    SelectionList       &selection() override;
    SelectionList const &selection() const override;

  protected:
    void showEvent(QShowEvent *event) override;

  Q_SIGNALS:
    void itemActivated(qsizetype) override;
    void thumbnailsRequested(SelectionList, int, bool, bool) override;
    void draggedOut() override;
    void draggedToBookmarks(SelectionList) override;
    void sortingSelected(SortingMode);
    void showFoldersChanged(bool mode);
    void directorySelected(QString);
    void copyUrlsRequested(QList<QString>, QString path);
    void moveUrlsRequested(QList<QString>, QString path);
    void droppedInto(QMimeData const *, QObject *, qsizetype) override;
    void draggedOver(qsizetype) override;

  private:
    FolderView  *folderView;
    QVBoxLayout *layout;
    StateBuffer  stateBuf;
    QMutex       mtx;
};

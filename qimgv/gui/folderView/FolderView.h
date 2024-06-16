#pragma once

#include "gui/customWidgets/ActionButton.h"
#include "gui/customWidgets/FloatingWidgetContainer.h"
#include "gui/customWidgets/StyledComboBox.h"
#include "gui/folderView/FileSystemModelCustom.h"
#include "gui/folderView/BookmarksWidget.h"
#include "gui/folderView/FolderGridView.h"
#include "gui/folderView/FVOptionsPopup.h"
#include "gui/IDirectoryView.h"
#include <QAbstractItemView>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QStyledItemDelegate>
#include <QWidget>

namespace Ui {
class FolderView;
}

class FolderView : public FloatingWidgetContainer, public IDirectoryView
{
    Q_OBJECT
    Q_INTERFACES(IDirectoryView)

  public:
    explicit FolderView(QWidget *parent);
    ~FolderView() override;

    SelectionList       &selection() final;
    SelectionList const &selection() const final;

  public Q_SLOTS:
    void show();
    void hide();
    void populate(qsizetype) override;
    void setThumbnail(qsizetype pos, QSharedPointer<Thumbnail> thumb) override;
    void select(SelectionList) override;
    void select(qsizetype index) override;
    void focusOn(qsizetype index) override;
    void focusOnSelection() override;
    void setDirectoryPath(QString path) override;
    void insertItem(qsizetype index) override;
    void removeItem(qsizetype index) override;
    void reloadItem(qsizetype index) override;
    void setDragHover(qsizetype) override;
    void addItem() const;
    void onFullscreenModeChanged(bool mode) const;
    void onSortingChanged(SortingMode mode) const;

  protected:
    void wheelEvent(QWheelEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *event) override;

  protected Q_SLOTS:
    void onThumbnailSizeChanged(int newSize) const;
    void onZoomSliderValueChanged(int value) const;

  Q_SIGNALS:
    void itemActivated(qsizetype) override;
    void thumbnailsRequested(SelectionList, int, bool, bool) override;
    void draggedOut() override;
    void draggedToBookmarks(SelectionList) override;
    void sortingSelected(SortingMode);
    void directorySelected(QString path);
    void showFoldersChanged(bool mode);
    void copyUrlsRequested(QList<QString>, QString path);
    void moveUrlsRequested(QList<QString>, QString path);
    void droppedInto(QMimeData const *, QObject *, qsizetype) override;
    void draggedOver(qsizetype) override;

  private Q_SLOTS:
    void onSortingSelected(int);
    void readSettings() const;

    void onTreeViewClicked(QModelIndex const &index);
    void onDroppedInByIndex(QList<QString> const &, QModelIndex const &index);
    void toggleBookmarks() const;
    void toggleFilesystemView() const;
    void setPlacesPanel(bool mode) const;
    void onPlacesPanelButtonChecked(bool mode) const;
    void onBookmarkClicked(QString const &dirPath);
    void newBookmark();
    void fsTreeScrollToCurrent() const;

    void onOptionsPopupButtonToggled(bool mode) const;
    void onOptionsPopupDismissed();
    void onViewModeSelected(FolderViewMode mode);

    void onSplitterMoved() const;
    void onHomeBtn();
    void onRootBtn();
    void onTreeViewTabOut() const;

  private:
    Ui::FolderView        *ui;
    FileSystemModelCustom *dirModel;
    FVOptionsPopup        *optionsPopup;
    QElapsedTimer          popupTimerClutch;
};

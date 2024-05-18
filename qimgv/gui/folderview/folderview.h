#pragma once

#include <QAbstractItemView>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QStyledItemDelegate>
#include <QWidget>
#include "gui/customwidgets/actionbutton.h"
#include "gui/customwidgets/floatingwidgetcontainer.h"
#include "gui/customwidgets/styledcombobox.h"
#include "gui/folderview/bookmarkswidget.h"
#include "gui/folderview/FileSystemModelCustom.h"
#include "gui/folderview/foldergridview.h"
#include "gui/folderview/fvoptionspopup.h"
#include "gui/idirectoryview.h"

namespace Ui {
class FolderView;
}

class FolderView : public FloatingWidgetContainer, public IDirectoryView
{
    Q_OBJECT
    Q_INTERFACES(IDirectoryView)
  public:
    explicit FolderView(QWidget *parent = nullptr);
    ~FolderView() override;

  public slots:
    void show();
    void hide();

    void populate(int) override;
    void setThumbnail(int pos, std::shared_ptr<Thumbnail> thumb) override;
    void select(QList<int>) override;
    void select(int index) override;
    void focusOn(int index) override;
    void focusOnSelection() override;
    void setDirectoryPath(QString path) override;
    void insertItem(int index) override;
    void removeItem(int index) override;
    void reloadItem(int index) override;
    void setDragHover(int) override;
    void addItem() const;
    void onFullscreenModeChanged(bool mode) const;
    void onSortingChanged(SortingMode mode) const;

    QList<int> &      selection() override;
    QList<int> const &selection() const override;

  protected:
    void wheelEvent(QWheelEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *event) override;

  protected slots:
    void onThumbnailSizeChanged(int newSize) const;
    void onZoomSliderValueChanged(int value) const;

  signals:
    void itemActivated(int) override;
    void thumbnailsRequested(QList<int>, int, bool, bool) override;
    void draggedOut() override;
    void draggedToBookmarks(QList<int>) override;
    void sortingSelected(SortingMode);
    void directorySelected(QString path);
    void showFoldersChanged(bool mode);
    void copyUrlsRequested(QList<QString>, QString path);
    void moveUrlsRequested(QList<QString>, QString path);
    void droppedInto(const QMimeData *, QObject *, int) override;
    void draggedOver(int) override;

  private slots:
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

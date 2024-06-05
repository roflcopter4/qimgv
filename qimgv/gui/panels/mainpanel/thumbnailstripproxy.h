#pragma once

#include "gui/panels/mainpanel/thumbnailstrip.h"
#include <QMutexLocker>
#include "Common.h"

class ThumbnailStripProxy : public QWidget, public IDirectoryView
{
    Q_OBJECT
    Q_INTERFACES(IDirectoryView)

    struct ThumbnailStripStateBuffer {
        SelectionList selection{};
        qsizetype     itemCount = 0;
    };

  public:
    explicit ThumbnailStripProxy(QWidget *parent = nullptr);
    ~ThumbnailStripProxy() override;

    void init();
    void readSettings();

    ND bool  isInitialized() const;
    ND QSize itemSize() const;

  public Q_SLOTS:
    void populate(qsizetype) override;
    void setThumbnail(qsizetype pos, QSharedPointer<Thumbnail> thumb) override;
    void select(SelectionList) override;
    void select(qsizetype index) override;
    void focusOn(qsizetype index) override;
    void focusOnSelection() override;
    void insertItem(qsizetype index) override;
    void removeItem(qsizetype index) override;
    void reloadItem(qsizetype index) override;
    void setDragHover(qsizetype index) override;
    void setDirectoryPath(QString path) override;
    void addItem();

    ND SelectionList       &selection() final;
    ND SelectionList const &selection() const final;

  protected:
    void showEvent(QShowEvent *event) override;

  Q_SIGNALS:
    void itemActivated(qsizetype) override;
    void thumbnailsRequested(SelectionList, int, bool, bool) override;
    void draggedOut() override;
    void draggedToBookmarks(SelectionList) override;
    void droppedInto(QMimeData const *, QObject *, qsizetype) override;
    void draggedOver(qsizetype) override;

  private:
    ThumbnailStrip *thumbnailStrip = nullptr;

    QVBoxLayout               *layout;
    ThumbnailStripStateBuffer stateBuf{};
    QMutex                    m{};
};

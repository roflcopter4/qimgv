#pragma once

#include "gui/panels/mainpanel/thumbnailstrip.h"
#include <QMutexLocker>
#include "Common.h"

struct ThumbnailStripStateBuffer {
    QList<int> selection;
    qsizetype  itemCount = 0;
};

class ThumbnailStripProxy : public QWidget, public IDirectoryView
{
    Q_OBJECT
    Q_INTERFACES(IDirectoryView)

  public:
    explicit ThumbnailStripProxy(QWidget *parent = nullptr);

    void init();
    void readSettings();

    ND bool  isInitialized() const;
    ND QSize itemSize() const;

  public Q_SLOTS:
    void populate(int) override;
    void setThumbnail(int pos, std::shared_ptr<Thumbnail> thumb) override;
    void select(QList<int>) override;
    void select(int index) override;
    void focusOn(int index) override;
    void focusOnSelection() override;
    void insertItem(int index) override;
    void removeItem(int index) override;
    void reloadItem(int index) override;
    void setDragHover(int index) override;
    void setDirectoryPath(QString path) override;
    void addItem();

    QList<int> &      selection() override;
    QList<int> const &selection() const override;

  protected:
    void showEvent(QShowEvent *event) override;

  Q_SIGNALS:
    void itemActivated(int) override;
    void thumbnailsRequested(QList<int>, int, bool, bool) override;
    void draggedOut() override;
    void draggedToBookmarks(QList<int>) override;
    void droppedInto(QMimeData const *, QObject *, int) override;
    void draggedOver(int) override;

  private:
    std::shared_ptr<ThumbnailStrip> thumbnailStrip = nullptr;
    QVBoxLayout                     layout;
    ThumbnailStripStateBuffer       stateBuf;
    QMutex                          m;
};

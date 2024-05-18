#pragma once


#include <QList>
#include <QtPlugin>
#include <QtTypes>
#include <QtClassHelperMacros>
#include <memory>

class Thumbnail;
class QString;
class QMimeData;

class IDirectoryView
{
  public:
    virtual ~IDirectoryView() = default;

    virtual void populate(int)                                           = 0;
    virtual void setThumbnail(int pos, std::shared_ptr<Thumbnail> thumb) = 0;
    virtual void select(QList<int>)                                      = 0;
    virtual void select(int index)                                       = 0;
    virtual void focusOn(int index)                                      = 0;
    virtual void focusOnSelection()                                      = 0;
    virtual void setDirectoryPath(QString path)                          = 0;
    virtual void insertItem(int index)                                   = 0;
    virtual void removeItem(int index)                                   = 0;
    virtual void reloadItem(int index)                                   = 0;
    virtual void setDragHover(int index)                                 = 0;

    ND virtual QList<int>       &selection()       = 0;
    ND virtual QList<int> const &selection() const = 0;

    // signals
    virtual void itemActivated(int)                               = 0;
    virtual void thumbnailsRequested(QList<int>, int, bool, bool) = 0;
    virtual void draggedOut()                                     = 0;
    virtual void draggedToBookmarks(QList<int>)                   = 0;
    virtual void draggedOver(int)                                 = 0;
    virtual void droppedInto(QMimeData const *, QObject *, int)   = 0;
};

Q_DECLARE_INTERFACE(IDirectoryView, "IDirectoryView")

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
    using SelectionList = QList<qsizetype>;

    virtual ~IDirectoryView() = default;

    virtual void populate(qsizetype)            = 0;
    virtual void select(SelectionList)          = 0;
    virtual void select(qsizetype index)        = 0;
    virtual void focusOn(qsizetype index)       = 0;
    virtual void focusOnSelection()             = 0;
    virtual void setDirectoryPath(QString path) = 0;
    virtual void insertItem(qsizetype index)    = 0;
    virtual void removeItem(qsizetype index)    = 0;
    virtual void reloadItem(qsizetype index)    = 0;
    virtual void setDragHover(qsizetype index)  = 0;

    virtual void setThumbnail(qsizetype pos, std::shared_ptr<Thumbnail> thumb) = 0;

    ND virtual SelectionList       &selection()       = 0;
    ND virtual SelectionList const &selection() const = 0;

    // signals
    virtual void itemActivated(qsizetype)                             = 0;
    virtual void thumbnailsRequested(SelectionList, int, bool, bool)  = 0;
    virtual void draggedOut()                                         = 0;
    virtual void draggedToBookmarks(SelectionList)                    = 0;
    virtual void draggedOver(qsizetype)                               = 0;
    virtual void droppedInto(QMimeData const *, QObject *, qsizetype) = 0;
};

Q_DECLARE_INTERFACE(IDirectoryView, "IDirectoryView")

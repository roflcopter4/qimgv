#pragma once

#include <QStackedWidget>
#include "gui/folderview/folderviewproxy.h"
#include "gui/viewers/documentwidget.h"
#include "settings.h"


class CentralWidget : public QStackedWidget
{
    Q_OBJECT
public:
    explicit CentralWidget(std::shared_ptr<DocumentWidget> const &aDocWidget,
                           std::shared_ptr<FolderViewProxy> const &aFolderView,
                           QWidget *parent = nullptr);

    ViewMode currentViewMode() const;
signals:

public slots:
    void showDocumentView();
    void showFolderView();
    void toggleViewMode();

private:
    std::shared_ptr<DocumentWidget> documentView;
    std::shared_ptr<FolderViewProxy> folderView;
    ViewMode mode;
};

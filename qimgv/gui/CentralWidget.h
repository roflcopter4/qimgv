#pragma once

#include <QStackedWidget>
#include "gui/folderView/FolderViewProxy.h"
#include "gui/viewers/DocumentWidget.h"
#include "Settings.h"


class CentralWidget : public QStackedWidget
{
    Q_OBJECT

  public:
    explicit CentralWidget(DocumentWidget *docWidget, FolderViewProxy *folderView, QWidget *parent);
    ~CentralWidget() override;
    DELETE_COPY_MOVE_ROUTINES(CentralWidget);

    ND ViewMode currentViewMode() const;

  public Q_SLOTS:
    void showDocumentView();
    void showFolderView();
    void toggleViewMode();

  private:
    DocumentWidget  *documentView;
    FolderViewProxy *folderView;
    ViewMode         mode = ViewMode::INVALID;
};

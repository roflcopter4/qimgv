#pragma once

#include <QStackedWidget>
#include "gui/folderView/FolderViewProxy.h"
#include "gui/viewers/DocumentWidget.h"
#include "Settings.h"


class CentralWidget : public QStackedWidget
{
    Q_OBJECT

  public:
    explicit CentralWidget(DocumentWidget *DocWidget, FolderViewProxy *aFolderView, QWidget *parent);

    ~CentralWidget() override;

    ViewMode currentViewMode() const;

  public slots:
    void showDocumentView();
    void showFolderView();
    void toggleViewMode();

  private:
    DocumentWidget  *documentView;
    FolderViewProxy *folderView;
    ViewMode         mode;
};

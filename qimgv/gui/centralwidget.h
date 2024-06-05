#pragma once

#include <QStackedWidget>
#include "gui/folderview/folderviewproxy.h"
#include "gui/viewers/documentwidget.h"
#include "settings.h"


class CentralWidget : public QStackedWidget
{
    Q_OBJECT

  public:
    explicit CentralWidget(DocumentWidget *DocWidget, FolderViewProxy *aFolderView, QWidget *parent = nullptr);

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

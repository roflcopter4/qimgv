#include "CentralWidget.h"

CentralWidget::CentralWidget(DocumentWidget  *docWidget,
                             FolderViewProxy *folderView,
                             QWidget         *parent)
    : QStackedWidget(parent),
      documentView(docWidget),
      folderView(folderView)
{
    setMouseTracking(true);
    if (!documentView || !folderView)
        qFatal() << u"[CentralWidget] Error: Child widget is null. We will crash now. Bye.";

    // docWidget - 0, folderView - 1
    addWidget(documentView);
    if (folderView)
        addWidget(folderView);
    showDocumentView();
}

CentralWidget::~CentralWidget() = default;

void CentralWidget::showDocumentView()
{
    if (mode == ViewMode::DOCUMENT)
        return;
    mode = ViewMode::DOCUMENT;
    setCurrentIndex(0);
    widget(0)->setFocus();
    documentView->viewWidget()->startPlayback();
}

void CentralWidget::showFolderView()
{
    if (mode == ViewMode::FOLDERVIEW)
        return;

    mode = ViewMode::FOLDERVIEW;
    setCurrentIndex(1);
    widget(1)->show();
    widget(1)->setFocus();
    documentView->viewWidget()->stopPlayback();
}

void CentralWidget::toggleViewMode()
{
    mode == ViewMode::DOCUMENT ? showFolderView() : showDocumentView();
}

ViewMode CentralWidget::currentViewMode() const
{
    return mode;
}

#pragma once

#include "gui/customWidgets/FloatingWidgetContainer.h"
#include "gui/panels/infoBar/InfoBarProxy.h"
#include "gui/panels/mainPanel/MainPanel.h"
#include "gui/viewers/ViewerWidget.h"
#include <QBoxLayout>
#include <QObject>

// TODO: use a template here?

class DocumentWidget : public FloatingWidgetContainer
{
    Q_OBJECT

  public:
    DocumentWidget(ViewerWidget *viewWidget, InfoBarProxy *infoBar, QWidget *parent);
    ~DocumentWidget() override;
    DELETE_COPY_MOVE_ROUTINES(DocumentWidget);

    ND auto viewWidget() const -> ViewerWidget *;
    ND auto thumbPanel() const -> ThumbnailStripProxy *;
    ND bool panelEnabled() const;

    void setFocus();
    void hideFloatingPanel();
    void hideFloatingPanel(bool animated);
    void setPanelEnabled(bool mode);
    void setupMainPanel();
    void setInteractionEnabled(bool mode);
    void allowPanelInit();

  protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

  public Q_SLOTS:
    void onFullscreenModeChanged(bool mode);

  private Q_SLOTS:
    void readSettings();
    void setPanelPinned(bool mode);
    ND bool panelPinned() const;

  private:
    QBoxLayout   *layout;
    QBoxLayout   *layoutRoot;
    ViewerWidget *mViewWidget;
    InfoBarProxy *mInfoBar;
    MainPanel    *mainPanel;

    bool mAllowPanelInit      : 1 = false;
    bool mAvoidPanelFlag      : 1 = false;
    bool mInteractionEnabled  : 1 = false;
    bool mIsFullscreen        : 1 = false;
    bool mPanelEnabled        : 1 = false;
    bool mPanelFullscreenOnly : 1 = false;
    bool mPanelPinned         : 1 = false;
};

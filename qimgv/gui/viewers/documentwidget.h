#pragma once

#include <memory>
#include <QBoxLayout>
#include "gui/customwidgets/floatingwidgetcontainer.h"
#include "gui/viewers/viewerwidget.h"
#include "gui/panels/mainpanel/mainpanel.h"
#include "gui/panels/infobar/infobarproxy.h"

// TODO: use a template here?

class DocumentWidget : public FloatingWidgetContainer
{
  public:
    DocumentWidget(QSharedPointer<ViewerWidget> const &viewWidget, QSharedPointer<InfoBarProxy> const &infoBar, QWidget*parent = nullptr);

    ~DocumentWidget() override;

    QSharedPointer<ViewerWidget>       &viewWidget();
    QSharedPointer<ThumbnailStripProxy> thumbPanel() const;

    void setFocus();
    void hideFloatingPanel();
    void hideFloatingPanel(bool animated);
    void setPanelEnabled(bool mode);
    bool panelEnabled() const;
    void setupMainPanel();
    void setInteractionEnabled(bool mode);
    void allowPanelInit();

  public slots:
    void onFullscreenModeChanged(bool mode);

  private slots:
    void setPanelPinned(bool mode);
    bool panelPinned() const;
    void readSettings();

  protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

  private:
    QBoxLayout                   *layout, *layoutRoot;
    QSharedPointer<ViewerWidget> mViewWidget;
    QSharedPointer<InfoBarProxy> mInfoBar;
    QSharedPointer<MainPanel>    mainPanel;

    bool avoidPanelFlag       : 1;
    bool mPanelEnabled        : 1;
    bool mPanelFullscreenOnly : 1;
    bool mIsFullscreen        : 1;
    bool mPanelPinned         : 1;
    bool mInteractionEnabled  : 1;
    bool mAllowPanelInit      : 1;
};

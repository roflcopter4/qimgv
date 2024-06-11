#pragma once

#include "Settings.h"
#include <QDebug>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QScrollBar>
#include <QTreeView>

class TreeViewCustom : public QTreeView
{
    Q_OBJECT

  public:
    explicit TreeViewCustom(QWidget *parent);

    ND QSize minimumSizeHint() const override;

  Q_SIGNALS:
    void droppedIn(QList<QString>, QModelIndex);
    void tabbedOut();

  protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

  private Q_SLOTS:
    void updateScrollbarStyle();
    void onValueChanged();

  private:
    QScrollBar overlayScrollbar;

    static constexpr int SCROLLBAR_WIDTH = 12;
};

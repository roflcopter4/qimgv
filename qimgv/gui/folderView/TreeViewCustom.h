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

  protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

  private:
    void reallyExpandRecursively(QModelIndex const &index);

  Q_SIGNALS:
    void droppedIn(QList<QString>, QModelIndex);
    void tabbedOut();

  private Q_SLOTS:
    void updateScrollbarStyle();
    void onValueChanged();
    //void continueExpansion(QModelIndex const &index);

private:
    QScrollBar overlayScrollbar;
    std::atomic_flag isExpanding;

    static constexpr int SCROLLBAR_WIDTH = 12;
};

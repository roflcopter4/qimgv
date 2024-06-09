#pragma once
#ifndef TREEVIEWCUSTOM_H
#define TREEVIEWCUSTOM_H

#include <QTreeView>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QScrollBar>
#include <QDebug>
#include "Settings.h"

#include "Common.h"

class TreeViewCustom : public QTreeView
{
    Q_OBJECT

  public:
    explicit TreeViewCustom(QWidget *parent);

    ND QSize minimumSizeHint() const override;

  signals:
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

  private slots:
    void updateScrollbarStyle();

  private:
    QScrollBar overlayScrollbar;

    static constexpr int SCROLLBAR_WIDTH = 12;
};

#endif // TREEVIEWCUSTOM_H

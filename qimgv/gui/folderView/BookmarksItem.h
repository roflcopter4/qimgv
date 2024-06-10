#pragma once

#include "Settings.h"
#include "gui/customWidgets/ClickableLabel.h"
#include "gui/customWidgets/IconButton.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QObject>
#include <QSpacerItem>
#include <QStyleOption>
#include <QWidget>

class BookmarksItem : public QWidget
{
    Q_OBJECT

  public:
    explicit BookmarksItem(QString const &dirName, QString const &dirPath, QWidget *parent = nullptr);

    ND QString path() const;

  public Q_SLOTS:
    void setHighlighted(bool mode);

  Q_SIGNALS:
    void clicked(QString dirPath);
    void removeClicked(QString dirPath);
    void droppedIn(QList<QString> paths, QString dirPath);

  protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

    void paintEvent(QPaintEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;

  private Q_SLOTS:
    void onRemoveClicked();

  private:
    QString dirName;
    QString dirPath;

    IconWidget   folderIconWidget;
    IconButton   removeItemButton;
    QLabel       dirNameLabel;
    QSpacerItem *spacer;
    QHBoxLayout *layout;
    bool         mHighlighted;
};

#pragma once

#include "Settings.h"
#include "gui/folderView/BookmarksItem.h"
#include <QLabel>
#include <QMimeData>
#include <QVBoxLayout>
#include <QWidget>

class BookmarksWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit BookmarksWidget(QWidget *parent = nullptr);
    ~BookmarksWidget() override;

  public Q_SLOTS:
    void addBookmark(QString const &directoryPath);
    void removeBookmark(QString const &dirPath);
    void onPathChanged(QString const &path);

  private Q_SLOTS:
    void readSettings();
    void saveBookmarks();

  Q_SIGNALS:
    void bookmarkClicked(QString dirPath);
    void droppedIn(QList<QString> paths, QString dirPath);

  protected:
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;

  private:
    QVBoxLayout *layout;
    QStringList  paths;
    QString      highlightedPath;
};

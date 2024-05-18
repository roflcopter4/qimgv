#ifndef BOOKMARKSWIDGET_H
#define BOOKMARKSWIDGET_H

#include <QWidget>
#include "gui/folderview/bookmarksitem.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QMimeData>
#include "settings.h"

class BookmarksWidget : public QWidget {
    Q_OBJECT

public:
    explicit BookmarksWidget(QWidget *parent = nullptr);
    ~BookmarksWidget() override;

public slots:
    void addBookmark(QString const &directoryPath);

    void removeBookmark(QString const &dirPath);
    void onPathChanged(QString const &path);
private slots:
    void readSettings();

    void saveBookmarks();
signals:
    void bookmarkClicked(QString dirPath);
    void droppedIn(QList<QString> paths, QString dirPath);

protected:
    void dropEvent(QDropEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
private:
    QVBoxLayout layout;
    QStringList paths;
    QString highlightedPath;
};

#endif // BOOKMARKSWIDGET_H

#ifndef FILESYSTEMMODELCUSTOM_H
#define FILESYSTEMMODELCUSTOM_H

#include <QApplication>
#include <QFileSystemModel>
#include <QPainter>
#include "settings.h"
#include "utils/imagelib.h"

class FileSystemModelCustom final : public QFileSystemModel
{
  public:
    explicit FileSystemModelCustom(QObject *parent = nullptr);

    ND QVariant data(QModelIndex const &index, int role) const override;

  protected:
    ND Qt::ItemFlags flags(QModelIndex const &index) const override;

#if 0
    bool event(QEvent *event) override;
#endif

  private:
    QPixmap folderIcon;
};

#endif // FILESYSTEMMODELCUSTOM_H

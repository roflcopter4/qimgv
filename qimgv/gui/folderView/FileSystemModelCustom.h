#pragma once

#include "Settings.h"
#include "utils/ImageLib.h"
#include <QApplication>
#include <QFileSystemModel>
#include <QPainter>

class FileSystemModelCustom final : public QFileSystemModel
{
    class FileIconProvider;

  public:
    explicit FileSystemModelCustom(QObject *parent = nullptr);
    ~FileSystemModelCustom() override;

    ND QVariant data(QModelIndex const &index, int role) const override;

  protected:
    ND Qt::ItemFlags flags(QModelIndex const &index) const override;

#if 0
    bool event(QEvent *event) override;
#endif

  private:
    QPixmap folderIcon;
    std::unique_ptr<FileIconProvider> iconProvider;
};

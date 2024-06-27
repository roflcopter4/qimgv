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

  private Q_SLOTS:
    void onSettingsChanged();

  private:
    QPixmap folderIcon;
    std::unique_ptr<FileIconProvider> iconProvider;
};

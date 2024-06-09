#pragma once

#include <QImage>
#include <QPixmap>
#include <QProcess>
#include <QFile>
#include "image.h"

class Video : public Image
{
  public:
    explicit Video(QString const &path);
    explicit Video(std::unique_ptr<DocumentInfo> info);

    ND auto  getPixmap() -> std::unique_ptr<QPixmap> override;
    ND auto  getImage() -> QSharedPointer<QImage const> override;
    ND int   height() const override;
    ND int   width() const override;
    ND QSize size() const override;

  public Q_SLOTS:
    bool save() override;
    bool save(QString destPath) override;

  private:
    void load() override;

    int srcWidth  = 0;
    int srcHeight = 0;
};

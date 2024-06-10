#pragma once

#include <QImage>
#include <QImageWriter>
#include <QSemaphore>
#include <QCryptographicHash>
#include <QIcon>
#include "Image.h"
#include "utils/ImageLib.h"
#include "Settings.h"

class ImageStatic : public Image
{
  public:
    explicit ImageStatic(QString const &path);
    explicit ImageStatic(std::unique_ptr<DocumentInfo> info);
    ~ImageStatic() override;

    ND auto  getSourceImage() -> QSharedPointer<QImage const>;
    ND auto  getPixmap() -> std::unique_ptr<QPixmap> override;
    ND auto  getImage() -> QSharedPointer<QImage const> override;
    ND int   height() const override;
    ND int   width() const override;
    ND QSize size() const override;

    bool setEditedImage(std::unique_ptr<QImage const> imageEditedNew);
    bool discardEditedImage();

  public Q_SLOTS:
    void crop(QRect newRect);
    bool save() override;
    bool save(QString destPath) override;

  private:
    QSharedPointer<QImage const> image;
    QSharedPointer<QImage const> imageEdited;

    void load() override;
    void loadGeneric();
    void loadICO();

    ND static auto readImage(QString const &path, QByteArray const &format) -> std::unique_ptr<QImage const>;
    ND static auto generateHash(QString const &str) -> QString;
};

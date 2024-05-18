#pragma once

#include <QImage>
#include <QImageWriter>
#include <QSemaphore>
#include <QCryptographicHash>
#include "image.h"
#include "utils/imagelib.h"
#include <settings.h>
#include <QIcon>

class ImageStatic : public Image
{
  public:
    ImageStatic(QString const &path);
    ImageStatic(std::unique_ptr<DocumentInfo> info);
    ~ImageStatic() override;

    std::unique_ptr<QPixmap>      getPixmap() override;
    std::shared_ptr<QImage const> getSourceImage();
    std::shared_ptr<QImage const> getImage() override;

    int   height() override;
    int   width() override;
    QSize size() override;

    bool setEditedImage(std::unique_ptr<QImage const> imageEditedNew);
    bool discardEditedImage();

  public Q_SLOTS:
    void crop(QRect newRect);
    bool save() override;
    bool save(QString destPath) override;

  private:
    std::shared_ptr<QImage const> image;
    std::shared_ptr<QImage const> imageEdited;

    void load() override;
    void loadGeneric();
    void loadICO();
    static QString generateHash(QString const &str);
};

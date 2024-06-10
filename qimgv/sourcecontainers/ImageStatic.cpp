#include "ImageStatic.h"
#include <time.h>

ImageStatic::ImageStatic(QString const &path)
    : Image(path)
{
    ImageStatic::load();
}

ImageStatic::ImageStatic(std::unique_ptr<DocumentInfo> info)
    : Image(std::move(info))
{
    ImageStatic::load();
}

ImageStatic::~ImageStatic() = default;

// load image data from disk
void ImageStatic::load()
{
    if (isLoaded())
        return;
    if (mDocInfo->mimeType().name() == QSV("image/vnd.microsoft.icon"))
        loadICO();
    else
        loadGeneric();
}

std::unique_ptr<QImage const>
ImageStatic::readImage(QString const &path, QByteArray const &format)
{
    /* QImageReader::read() seems more reliable than just reading via QImage.
     * For example: "Invalid JPEG file structure: two SOF markers"
     * QImageReader::read() returns false, but still reads an image. Meanwhile, QImage just fails.
     * I haven't checked QImage's code, but it seems like it sees an exception
     * from libjpeg or whatever and just gives up on reading the file.
     *
     * tldr: QImage bad
     */
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QImageReader::setAllocationLimit(settings->memoryAllocationLimit());
#endif
    auto reader = QImageReader(path, format);
    auto tmp = new QImage();
    reader.read(tmp);
    return std::unique_ptr<QImage const>{tmp};
}

void ImageStatic::loadGeneric()
{
    auto img = readImage(mPath, mDocInfo->format());
    img = ImageLib::exifRotated(std::move(img), mDocInfo->exifOrientation());

    // Scaling this format via qt results in transparent background.
    // It is rare enough so lets just convert it to the closest working thing.
    if (img->format() == QImage::Format_Mono)
        image.reset(new QImage(img->convertToFormat(QImage::Format_Grayscale8)));
    else
        image.reset(img.release());

    mLoaded = true;
}

// TODO: move this out somewhere to use in other places
void ImageStatic::loadICO()
{
    // Big brain code. It's mostly for small ico files so whatever. I'mtx not patching Qt for this.
    QIcon icon(mPath);
    QSize maxSize(0, 0);
    for (QSize sz : icon.availableSizes())
        if (maxSize.width() < sz.width())
            maxSize = sz;
    image   = QSharedPointer<QImage const>(new QImage(icon.pixmap(maxSize).toImage()));
    mLoaded = true;
}

QString ImageStatic::generateHash(QString const &str)
{
    return QString::fromLatin1(QCryptographicHash::hash(str.toLatin1(), QCryptographicHash::Md5).toHex());
}

// TODO: move saving to directorymodel
bool ImageStatic::save(QString destPath)
{
    QString   tmpPath = destPath + u'_' + generateHash(destPath);
    QFileInfo fi(destPath);
    QString   ext = fi.suffix();

    // png compression note from libpng
    // Note that tests have shown that zlib compression levels 3-6 usually perform as well
    // as level 9 for PNG images, and do considerably fewer caclulations
    int quality = 95;
    if (ext.compare(QSV("png"), Qt::CaseInsensitive) == 0)
        quality = 30;
    else if (ext.compare(QSV("jpg"), Qt::CaseInsensitive) == 0 || ext.compare(QSV("jpeg"), Qt::CaseInsensitive) == 0)
        quality = settings->JPEGSaveQuality();

    bool backupExists = false, success = false, originalExists = false;

    if (QFile::exists(destPath))
        originalExists = true;

    // backup the original file if possible
    if (originalExists) {
        QFile::remove(tmpPath);
        if (!QFile::copy(destPath, tmpPath)) {
            qDebug() << u"ImageStatic::save() - Could not create file backup.";
            return false;
        }
        backupExists = true;
    }

    // save file
    if (isEdited()) {
        success = imageEdited->save(destPath, ext.toStdString().c_str(), quality);
        image.swap(imageEdited);
        discardEditedImage();
    } else {
        success = image->save(destPath, ext.toStdString().c_str(), quality);
    }

    if (backupExists) {
        if (success) {
            // everything ok - remove the backup
            QFile file(tmpPath);
            file.remove();
        } else if (originalExists) {
            // revert on fail
            QFile::remove(mDocInfo->filePath());
            QFile::copy(tmpPath, mDocInfo->filePath());
            QFile::remove(tmpPath);
        }
    }

    if (destPath == mPath && success)
        mDocInfo->refresh();
    return success;
}

bool ImageStatic::save()
{
    return save(mPath);
}

std::unique_ptr<QPixmap> ImageStatic::getPixmap()
{
    auto pix = std::make_unique<QPixmap>();
    isEdited() ? pix->convertFromImage(*imageEdited) : pix->convertFromImage(*image, Qt::NoFormatConversion);
    return pix;
}

QSharedPointer<QImage const> ImageStatic::getSourceImage()
{
    return image;
}

QSharedPointer<QImage const> ImageStatic::getImage()
{
    return isEdited() ? imageEdited : image;
}

int ImageStatic::height() const
{
    return isEdited() ? imageEdited->height() : image->height();
}

int ImageStatic::width() const
{
    return isEdited() ? imageEdited->width() : image->width();
}

QSize ImageStatic::size() const
{
    return isEdited() ? imageEdited->size() : image->size();
}

bool ImageStatic::setEditedImage(std::unique_ptr<QImage const> imageEditedNew)
{
    if (imageEditedNew && imageEditedNew->width() != 0) {
        discardEditedImage();
        imageEdited = QSharedPointer<QImage const>(imageEditedNew.release());
        mEdited     = true;
        return true;
    }
    return false;
}

bool ImageStatic::discardEditedImage()
{
    if (imageEdited) {
        imageEdited.reset();
        mEdited = false;
        return true;
    }
    return false;
}

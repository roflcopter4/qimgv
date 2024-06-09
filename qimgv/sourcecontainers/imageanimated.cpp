#include "imageanimated.h"
#include <QMessageBox>

// TODO: this class is kinda useless now. redesign?

ImageAnimated::ImageAnimated(QString const &path)
    : Image(path),
      mSize(0, 0)
{
    ImageAnimated::load();
}

ImageAnimated::ImageAnimated(std::unique_ptr<DocumentInfo> info)
    : Image(std::move(info)),
      mSize(0, 0)
{
    ImageAnimated::load();
}

void ImageAnimated::load()
{
    if (isLoaded())
        return;
    loadMovie();
    mLoaded = true;
}

void ImageAnimated::loadMovie()
{
    movie = QSharedPointer<QMovie>(new QMovie());
    movie->setFileName(mPath);
    movie->setFormat(mDocInfo->format().toStdString().c_str());
    movie->jumpToFrame(0);
    mSize       = movie->frameRect().size();
    mFrameCount = movie->frameCount();
}

int ImageAnimated::frameCount() const
{
    return mFrameCount;
}

// TODO: overwrite (self included)
bool ImageAnimated::save(QString destPath)
{
    static constexpr auto warning1 = QSV("Unable to save file.");
    static constexpr auto warning2 = QSV("Unable to save file. Perhaps the source file was deleted?");

    QFile file(mPath);

    if (file.exists()) {
        if (!file.copy(destPath)) {
            QMessageBox::warning(nullptr, QS("Error"), util::QStringViewToQString(warning1));
            qDebug() << warning1;
            return false;
        }
        qDebug() << destPath << this->filePath();
        if (destPath == this->filePath())
            mDocInfo->refresh();
        return true;
    }

    QMessageBox::warning(nullptr, QS("Error"), util::QStringViewToQString(warning2));
    qDebug() << warning2;
    return false;
}

bool ImageAnimated::save()
{
    // TODO
    return false;
}

// in case of gif returns current frame
std::unique_ptr<QPixmap> ImageAnimated::getPixmap()
{
    return std::make_unique<QPixmap>(mPath, mDocInfo->format());
}

QSharedPointer<QImage const> ImageAnimated::getImage()
{
    return QSharedPointer<QImage const>(new QImage (mPath, mDocInfo->format()));
}

QSharedPointer<QMovie> ImageAnimated::getMovie()
{
    if (movie == nullptr)
        loadMovie();
    return movie;
}

int ImageAnimated::height() const
{
    return mSize.height();
}

int ImageAnimated::width() const
{
    return mSize.width();
}

QSize ImageAnimated::size() const
{
    return mSize;
}

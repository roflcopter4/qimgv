#include "Image.h"

Image::Image(QString const &path)
    : mDocInfo(new DocumentInfo(path)),
      mLoaded(false),
      mEdited(false),
      mPath(path)
{
}

Image::Image(std::unique_ptr<DocumentInfo> info)
    : mDocInfo(std::move(info)),
      mLoaded(false),
      mEdited(false),
      mPath(mDocInfo->filePath())
{
}

QString Image::filePath() const
{
    return mPath;
}

bool Image::isLoaded() const
{
    return mLoaded;
}

DocumentType Image::type() const
{
    return mDocInfo->type();
}

QString Image::fileName() const
{
    return mDocInfo->fileName();
}

QString Image::baseName() const
{
    return mDocInfo->baseName();
}

bool Image::isEdited() const
{
    return mEdited;
}

qint64 Image::fileSize() const
{
    return mDocInfo->fileSize();
}

QDateTime Image::lastModified() const
{
    return mDocInfo->lastModified();
}

QMap<QString, QString> Image::getExifTags() const
{
    return mDocInfo->getExifTags();
}

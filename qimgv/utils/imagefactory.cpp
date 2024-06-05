#include "ImageFactory.h"

QSharedPointer<Image>
ImageFactory::createImage(QString const &path)
{
    std::unique_ptr<DocumentInfo> docInfo(new DocumentInfo(path));
    QSharedPointer<Image>        img = nullptr;

    if (docInfo->type() == DocumentType::NONE)
        qDebug() << u"[ImageFactory] Cannot load" << docInfo->filePath();
    else if (docInfo->type() == DocumentType::ANIMATED)
        img = QSharedPointer<ImageAnimated>(new ImageAnimated(std::move(docInfo)));
    else if (docInfo->type() == DocumentType::VIDEO)
        img = QSharedPointer<Video>(new Video(std::move(docInfo)));
    else
        img = QSharedPointer<ImageStatic>(new ImageStatic(std::move(docInfo)));

    return img;
}

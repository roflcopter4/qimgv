#include "imagefactory.h"

std::shared_ptr<Image>
ImageFactory::createImage(QString const &path)
{
    std::unique_ptr<DocumentInfo> docInfo(new DocumentInfo(path));
    std::shared_ptr<Image>        img = nullptr;

    if (docInfo->type() == DocumentType::NONE)
        qDebug() << QSV("ImageFactory: cannot load ") << docInfo->filePath();
    else if (docInfo->type() == DocumentType::ANIMATED)
        img.reset(new ImageAnimated(std::move(docInfo)));
    else if (docInfo->type() == DocumentType::VIDEO)
        img.reset(new Video(std::move(docInfo)));
    else
        img.reset(new ImageStatic(std::move(docInfo)));

    return img;
}

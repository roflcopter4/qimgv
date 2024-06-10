#pragma once

#include "utils/ImageLib.h"
#include "sourcecontainers/DocumentInfo.h"
#include "sourcecontainers/Image.h"
#include "sourcecontainers/ImageAnimated.h"
#include "sourcecontainers/ImageStatic.h"
#include "sourcecontainers/Video.h"

class ImageFactory
{
  public:
    static QSharedPointer<Image> createImage(QString const &path);
};

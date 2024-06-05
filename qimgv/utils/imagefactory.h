#pragma once

#include "utils/imagelib.h"
#include "sourcecontainers/documentinfo.h"
#include "sourcecontainers/image.h"
#include "sourcecontainers/imageanimated.h"
#include "sourcecontainers/imagestatic.h"
#include "sourcecontainers/video.h"

class ImageFactory
{
  public:
    static QSharedPointer<Image> createImage(QString const &path);
};

#ifndef SCALERREQUEST_H
#define SCALERREQUEST_H

#include <QPixmap>
#include "sourcecontainers/Image.h"
#include "Settings.h" // move enums somewhere else?

class ScalerRequest {
public:
    ScalerRequest() : image(nullptr), size(QSize(0,0)), filter(ScalingFilter::BILINEAR) { }
    ScalerRequest(QSharedPointer<Image> _image, QSize _size, QString _path, ScalingFilter _filter) : image(_image), size(_size), path(_path), filter(_filter) {}
    QSharedPointer<Image> image;
    QSize size;
    QString path;
    ScalingFilter filter;

    bool operator==(const ScalerRequest &another) const {
        if(another.image == image && another.size == size && another.filter == filter)
            return true;
        return false;
    }
};

#endif // SCALERREQUEST_H

#include "ImageLib.h"

namespace ImageLib {

void recolor(QPixmap &pixmap, QColor const &color)
{
    QPainter p(&pixmap);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.setBrush(color);
    p.setPen(color);
    p.drawRect(pixmap.rect());
}

QImage *rotatedRaw(QImage const *src, int grad)
{
    if (!src)
        return new QImage();
    auto      *img = new QImage();
    QTransform transform;
    transform.rotate(grad);
    *img = src->transformed(transform, Qt::SmoothTransformation);
    return img;
}

QImage *rotated(QSharedPointer<QImage const> const &src, int grad)
{
    return rotatedRaw(src.get(), grad);
}

QImage *croppedRaw(QImage const *src, QRect newRect)
{
    if (src && src->rect().contains(newRect, false)) {
        auto *img = new QImage(newRect.size(), src->format());
        *img      = src->copy(newRect);
        return img;
    }
    return new QImage();
}

QImage *cropped(QSharedPointer<QImage const> const &src, QRect newRect)
{
    return croppedRaw(src.get(), newRect);
}

QImage *flippedHRaw(QImage const *src)
{
    if (!src)
        return new QImage();
    return new QImage(src->mirrored(true, false));
}

QImage *flippedH(QSharedPointer<QImage const> const &src)
{
    return flippedHRaw(src.get());
}

QImage *flippedVRaw(QImage const *src)
{
    if (!src)
        return new QImage();
    return new QImage(src->mirrored(false, true));
}

QImage *flippedV(QSharedPointer<QImage const> const &src)
{
    return flippedVRaw(src.get());
}

std::unique_ptr<QImage const>
exifRotated(std::unique_ptr<QImage const> src, int orientation)
{
    switch (orientation) {
    case 1: {
        src.reset(flippedHRaw(src.get()));
    } break;
    case 2: {
        src.reset(flippedVRaw(src.get()));
    } break;
    case 3: {
        src.reset(flippedHRaw(src.get()));
        src.reset(flippedVRaw(src.get()));
    } break;
    case 4: {
        src.reset(rotatedRaw(src.get(), 90));
    } break;
    case 5: {
        src.reset(flippedHRaw(src.get()));
        src.reset(rotatedRaw(src.get(), 90));
    } break;
    case 6: {
        src.reset(flippedVRaw(src.get()));
        src.reset(rotatedRaw(src.get(), 90));
    } break;
    case 7: {
        src.reset(rotatedRaw(src.get(), -90));
    } break;
    default: {
    } break;
    }
    return src;
}

std::unique_ptr<QImage>
exifRotated(std::unique_ptr<QImage> src, int orientation)
{
    switch (orientation) {
    case 1: {
        src.reset(flippedHRaw(src.get()));
    } break;
    case 2: {
        src.reset(flippedVRaw(src.get()));
    } break;
    case 3: {
        src.reset(flippedHRaw(src.get()));
        src.reset(flippedVRaw(src.get()));
    } break;
    case 4: {
        src.reset(rotatedRaw(src.get(), 90));
    } break;
    case 5: {
        src.reset(flippedHRaw(src.get()));
        src.reset(rotatedRaw(src.get(), 90));
    } break;
    case 6: {
        src.reset(flippedVRaw(src.get()));
        src.reset(rotatedRaw(src.get(), 90));
    } break;
    case 7: {
        src.reset(rotatedRaw(src.get(), -90));
    } break;
    default: {
    } break;
    }
    return src;
}

#if 0
QImage *cropped(QRect newRect, QRect targetRes, bool upscaled) {
    QImage *cropped = new QImage(targetRes.size(), image->format());
    if(upscaled) {
        QImage temp = image->copy(newRect);
        *cropped = temp.scaled(targetRes.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        QRect target(QPoint(0, 0), targetRes.size());
        target.moveCenter(cropped->rect().center());
        *cropped = cropped->copy(target);
    } else {
        newRect.moveCenter(image->rect().center());
        *cropped = image->copy(newRect);
    }
    return cropped;
}
#endif

QImage *scaled(QSharedPointer<QImage const> const &source, QSize destSize, ScalingFilter filter)
{
    if (!source)
        return new QImage();
    auto scaleTarget = source;
    if (source->format() == QImage::Format_Indexed8) {
        auto newFmt = QImage::Format_RGB32;
        if (source->hasAlphaChannel())
            newFmt = QImage::Format_ARGB32;
        scaleTarget = QSharedPointer<QImage>(new QImage(source->convertToFormat(newFmt)));
    }
#ifdef USE_OPENCV
    if (filter > ScalingFilter::BILINEAR && !QtOcv::isSupported(scaleTarget->format()))
        filter = ScalingFilter::BILINEAR;
#endif
    switch (filter) {
    case ScalingFilter::NEAREST:
        return scaled_Qt(scaleTarget, destSize, false);
    case ScalingFilter::BILINEAR:
        return scaled_Qt(scaleTarget, destSize, true);
#ifdef USE_OPENCV
    case ScalingFilter::CV_BILINEAR_SHARPEN:
        return scaled_CV(scaleTarget, destSize, cv::INTER_LINEAR, 0);
    case ScalingFilter::CV_CUBIC:
        return scaled_CV(scaleTarget, destSize, cv::INTER_CUBIC, 0);
    case ScalingFilter::CV_CUBIC_SHARPEN:
        return scaled_CV(scaleTarget, destSize, cv::INTER_CUBIC, 1);
#endif
    default:
        return scaled_Qt(scaleTarget, destSize, true);
    }
}

QImage *scaled_Qt(QSharedPointer<QImage const> const &source, QSize destSize, bool smooth)
{
    if (!source)
        return new QImage();
    auto *dest = new QImage();
    Qt::TransformationMode mode = smooth ? Qt::SmoothTransformation : Qt::FastTransformation;
    *dest = source->scaled(destSize.width(), destSize.height(), Qt::IgnoreAspectRatio, mode);
    return dest;
}

#ifdef USE_OPENCV
// this probably leaks, needs checking
QImage *
scaled_CV(QSharedPointer<QImage const> const &source,
          QSize destSize, cv::InterpolationFlags filter, int sharpen)
{
    auto *dest = new QImage();
    if (!source)
        return dest;

    QtOcv::MatColorOrder order;
    cv::Mat  srcMat     = QtOcv::image2Mat_shared(*source, &order);
    cv::Size destSizeCv = cv::Size(destSize.width(), destSize.height());

    if (destSize == source->size()) {
        // TODO: should this return a copy?
        //result = QSharedPointer<StaticImageContainer>(new StaticImageContainer(std::make_shared<cv::Mat>(srcMat)));
    }
    else if (destSize.width() > source->width()) { // upscale
        cv::Mat dstMat(destSizeCv, srcMat.type());
        cv::resize(srcMat, dstMat, destSizeCv, 0, 0, filter);
        *dest = QtOcv::mat2Image(dstMat, order, source->format());
    }
    else { // downscale
        float scale = static_cast<float>(destSize.width()) / static_cast<float>(source->width());
        if (scale < 0.5f && filter != cv::INTER_NEAREST) {
            if (filter == cv::INTER_CUBIC)
                sharpen = 1;
            filter = cv::INTER_AREA;
        }

        cv::Mat dstMat(destSizeCv, srcMat.type());
        cv::resize(srcMat, dstMat, destSizeCv, 0, 0, filter);
        if (!sharpen || filter == cv::INTER_NEAREST) {
            *dest = QtOcv::mat2Image(dstMat, order, source->format());
        } else {
            // todo: tweak this
            double amount = 0.25 * sharpen;
            // unsharp mask
            cv::Mat dstMat_sharpened;
            cv::GaussianBlur(dstMat, dstMat_sharpened, cv::Size(0, 0), 2);
            cv::addWeighted(dstMat, 1.0 + amount, dstMat_sharpened, -amount, 0, dstMat_sharpened);
            *dest = QtOcv::mat2Image(dstMat_sharpened, order, source->format());
        }
    }

    return dest;
}
#endif

} // namespace ImageLib

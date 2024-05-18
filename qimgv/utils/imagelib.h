#pragma once
#include "settings.h"
#include "sourcecontainers/documentinfo.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QImage>
#include <QPainter>
#include <QPixmapCache>
#include <QProcess>
#include <memory>

#ifdef USE_OPENCV
# include "3rdparty/QtOpenCV/cvmatandqimage.h"
# include <opencv2/imgproc.hpp>
#endif

namespace ImageLib {

 QImage *rotatedRaw(QImage const *src, int grad);
 QImage *rotated(std::shared_ptr<QImage const> const &src, int grad);

 QImage *croppedRaw(QImage const *src, QRect newRect);
 QImage *cropped(std::shared_ptr<QImage const> const &src, QRect newRect);

 QImage *flippedHRaw(QImage const *src);
 QImage *flippedH(std::shared_ptr<QImage const> const &src);

 QImage *flippedVRaw(QImage const *src);
 QImage *flippedV(std::shared_ptr<QImage const> const &src);

// QImage *scaled(const QImage *source, QSize destSize, ScalingFilter filter);
 QImage *scaled(std::shared_ptr<QImage const> const &source, QSize destSize, ScalingFilter filter);

 QImage *scaled_Qt(QImage const *source, QSize destSize, bool smooth);
 QImage *scaled_Qt(std::shared_ptr<QImage const> const &source, QSize destSize, bool smooth);

#ifdef USE_OPENCV
 QImage *scaled_CV(std::shared_ptr<QImage const> const &source, QSize destSize, cv::InterpolationFlags filter, int sharpen);
#endif
 std::unique_ptr<QImage const> exifRotated(std::unique_ptr<QImage const> src, int orientation);
 std::unique_ptr<QImage>       exifRotated(std::unique_ptr<QImage> src, int orientation);
 void                          recolor(QPixmap &pixmap, QColor const &color);

}; // namespace ImageLib

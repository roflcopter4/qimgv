#include "ThumbnailerRunnable.h"

ThumbnailerRunnable::ThumbnailerRunnable(ThumbnailCache *cache, QString path, int size, bool crop, bool force)
    : cache(cache),
      path(std::move(path)),
      size(size),
      crop(crop),
      force(force)
{}

void ThumbnailerRunnable::run()
{
    emit taskStart(path, size);
    QSharedPointer<Thumbnail> thumbnail = generate(cache, path, size, crop, force);
    emit taskEnd(thumbnail, path);
}

QString ThumbnailerRunnable::generateIdString(QString const &path, int size, bool crop)
{
    QString queryStr = path + QString::number(size);
    if (crop)
        queryStr.append(u's');
    queryStr = u"%1"_s.arg(QString::fromLatin1(QCryptographicHash::hash(queryStr.toUtf8(), QCryptographicHash::Md5).toHex()));
    return queryStr;
}

QSharedPointer<Thumbnail>
// ReSharper disable once CppParameterMayBeConstPtrOrRef
ThumbnailerRunnable::generate(ThumbnailCache *cache, QString const &path, int size, bool crop, bool force)
{
    std::unique_ptr<QImage> image;

    DocumentInfo imgInfo(path);
    QString      thumbnailId = generateIdString(path, size, crop);

    QString time = QString::number(imgInfo.lastModified().toMSecsSinceEpoch());

    if (!force && cache) {
        image.reset(cache->readThumbnail(thumbnailId));
        if (image && image->text(u"lastModified"_s) != time)
            image.reset(nullptr);
    }

    if (!image) {
        if (imgInfo.type() == DocumentType::NONE) {
            QSharedPointer<Thumbnail> thumbnail(new Thumbnail(imgInfo.fileName(), u""_s, size, nullptr));
            return thumbnail;
        }
        std::pair<QImage *, QSize> pair;
        if (imgInfo.type() == DocumentType::VIDEO)
            pair = createVideoThumbnail(path, size, crop);
        else
            pair = createThumbnail(imgInfo.filePath(), imgInfo.format().toStdString().c_str(), size, crop);
        image.reset(pair.first);
        QSize originalSize = pair.second;

        image = ImageLib::exifRotated(std::move(image), imgInfo.exifOrientation());

        // put in image info
        image->setText(u"originalWidth"_s, QString::number(originalSize.width()));
        image->setText(u"originalHeight"_s, QString::number(originalSize.height()));
        image->setText(u"lastModified"_s, time);

        if (imgInfo.type() == DocumentType::ANIMATED)
            image->setText(u"label"_s, u" [a]"_s);
        else if (imgInfo.type() == DocumentType::VIDEO)
            image->setText(u"label"_s, u" [v]"_s);

        if (cache) {
            // save thumbnail if it makes sense
            // FIXME: avoid too much i/o
            if (originalSize.width() > size || originalSize.height() > size)
                cache->saveThumbnail(image.get(), thumbnailId);
        }
    }
    auto tmpPixmap = new QPixmap(image->size());
    *tmpPixmap     = QPixmap::fromImage(*image);
    tmpPixmap->setDevicePixelRatio(qApp->devicePixelRatio());

    QString label;
    if (tmpPixmap->width() == 0) {
        label = u"error"_s;
    } else {
        // put info into Thumbnail object
        label = image->text(u"originalWidth"_s) + u'x' + image->text(u"originalHeight"_s) + image->text(u"label"_s);
    }

    QSharedPointer<QPixmap>   pixmapPtr(tmpPixmap);
    QSharedPointer<Thumbnail> thumbnail(new Thumbnail(imgInfo.fileName(), label, size, pixmapPtr));
    return thumbnail;
}

std::pair<QImage *, QSize>
ThumbnailerRunnable::createThumbnail(QString const &path, char const *format, int size, bool crop)
{
    QImage *result       = nullptr;
    auto    reader       = std::make_unique<QImageReader>(path, format);
    bool    indexed      = reader->imageFormat() == QImage::Format_Indexed8;
    bool    manualResize = indexed || !reader->supportsOption(QImageIOHandler::Size);
    bool    resizeAnyway = false;
    auto    ARMode       = crop ? Qt::KeepAspectRatioByExpanding : Qt::KeepAspectRatio;
    QSize   originalSize;

    if (!manualResize) {
        // Resize during read via QImageReader (faster)
        QSize scaledSize = reader->size().scaled(size, size, ARMode);
        reader->setScaledSize(scaledSize);

        if (crop) {
            QRect clip(0, 0, size, size);
            QRect scaledRect(QPoint(0, 0), scaledSize);
            clip.moveCenter(scaledRect.center());
            reader->setScaledClipRect(clip);
        }
        originalSize = reader->size();
        result       = new QImage();

        if (!reader->read(result)) {
            // If read() returns false there's no guarantee that size conversion worked properly.
            // So we fall back to manual.
            // Se far I've seen this happen only on some weird (corrupted?) jpeg saved from camera
            resizeAnyway = true;
            delete result;
            result = nullptr;
            // Force reset reader because it is really finicky and can fail on the second read attempt (yeah wtf)
            reader->setFileName(QString());
            reader.reset(new QImageReader(path, format));
        }
    }

    if (manualResize || resizeAnyway) {
        // Manual resize & crop. slower but should just work
        auto fullSize = std::make_unique<QImage>();
        reader->read(fullSize.get());

        if (indexed) {
            auto newFmt = QImage::Format_RGB32;
            if (fullSize->hasAlphaChannel())
                newFmt = QImage::Format_ARGB32;
            fullSize.reset(new QImage(fullSize->convertToFormat(newFmt)));
        }
        originalSize     = fullSize->size();
        QSize scaledSize = fullSize->size().scaled(size, size, ARMode);

        if (crop) {
            QRect clip(0, 0, size, size);
            QRect scaledRect(QPoint(0, 0), scaledSize);
            clip.moveCenter(scaledRect.center());
            auto scaled = QImage(fullSize->scaled(scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            result      = ImageLib::croppedRaw(&scaled, clip);
        } else {
            result = new QImage(fullSize->scaled(scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        }
    }

    // Force reader to close the file so it can be deleted later.
    reader->setFileName(u""_s);
    return std::make_pair(result, originalSize);
}

std::pair<QImage *, QSize>
ThumbnailerRunnable::createVideoThumbnail(QString const &path, int size, bool crop)
{
    QFileInfo fi(path);
    QString   tmpFilePath    = settings->tmpDir() + fi.fileName() + u".png";
    QString   tmpFilePathEsc = tmpFilePath;
    tmpFilePathEsc.replace(u'%', u"%%"_s);

    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    QStringList argv = {
        u"--start=30%"_s,
        u"--frames=1"_s,
        u"--aid=no"_s,
        u"--sid=no"_s,
        u"--no-config"_s,
        u"--load-scripts=no"_s,
        u"--no-terminal"_s,
        u"--o=" + tmpFilePathEsc,
        path
    };
    process.start(settings->mpvBinary(), argv);
    process.waitForFinished(8000);
    process.close();

    auto reader = QImageReader(tmpFilePath, "png");
    auto ARMode = crop ? Qt::KeepAspectRatioByExpanding : Qt::KeepAspectRatio;

    // scale & crop
    QSize scaledSize = reader.size().scaled(size, size, ARMode);
    reader.setScaledSize(scaledSize);
    if (crop) {
        QRect clip(0, 0, size, size);
        QRect scaledRect(QPoint(0, 0), scaledSize);
        clip.moveCenter(scaledRect.center());
        reader.setScaledClipRect(clip);
    }
    QSize originalSize = reader.size();
    auto  result       = new QImage(reader.read());

    // Force reader to close the file so it can be deleted later.
    reader.setFileName(u""_s);

    // remove temporary file
    QFile tmpFile(tmpFilePath);
    tmpFile.remove();

    return std::make_pair(result, originalSize);
}

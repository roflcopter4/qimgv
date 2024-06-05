#include "thumbnailerrunnable.h"

ThumbnailerRunnable::ThumbnailerRunnable(ThumbnailCache *cache, QString path, int size, bool crop, bool force)
    : path(std::move(path)),
      size(size),
      crop(crop),
      force(force),
      cache(cache)
{}

void ThumbnailerRunnable::run() {
    emit taskStart(path, size);
    QSharedPointer<Thumbnail> thumbnail = generate(cache, path, size, crop, force);
    emit taskEnd(thumbnail, path);
}

QString ThumbnailerRunnable::generateIdString(QString const &path, int size, bool crop) {
    QString queryStr = path + QString::number(size);
    if(crop)
        queryStr.append(QS("s"));
    queryStr = QS("%1").arg(QString::fromLatin1(QCryptographicHash::hash(queryStr.toUtf8(),QCryptographicHash::Md5).toHex()));
    return queryStr;
}

QSharedPointer<Thumbnail> ThumbnailerRunnable::generate(ThumbnailCache*cache, QString const &path, int size, bool crop, bool force) {
    DocumentInfo imgInfo(path);
    QString thumbnailId = generateIdString(path, size, crop);
    std::unique_ptr<QImage> image;

    QString time = QString::number(imgInfo.lastModified().toMSecsSinceEpoch());

    if(!force && cache) {
        image.reset(cache->readThumbnail(thumbnailId));
        if(image && image->text(QS("lastModified")) != time)
            image.reset(nullptr);
    }

    if(!image) {
        if(imgInfo.type() == DocumentType::NONE) {
            QSharedPointer<Thumbnail> thumbnail(new Thumbnail(imgInfo.fileName(), QS(""), size, nullptr));
            return thumbnail;
        }
        std::pair<QImage*, QSize> pair;
        if(imgInfo.type() == DocumentType::VIDEO)
            pair = createVideoThumbnail(path, size, crop);
        else
            pair = createThumbnail(imgInfo.filePath(), imgInfo.format().toStdString().c_str(), size, crop);
        image.reset(pair.first);
        QSize originalSize = pair.second;

        image = ImageLib::exifRotated(std::move(image), imgInfo.exifOrientation());

        // put in image info
        image->setText(QS("originalWidth"), QString::number(originalSize.width()));
        image->setText(QS("originalHeight"), QString::number(originalSize.height()));
        image->setText(QS("lastModified"), time);

        if(imgInfo.type() == DocumentType::ANIMATED)
            image->setText(QS("label"), QS(" [a]"));
        else if(imgInfo.type() == DocumentType::VIDEO)
            image->setText(QS("label"), QS(" [v]"));

        if(cache) {
            // save thumbnail if it makes sense
            // FIXME: avoid too much i/o
            if(originalSize.width() > size || originalSize.height() > size)
                cache->saveThumbnail(image.get(), thumbnailId);
        }
    }
    auto && tmpPixmap = new QPixmap(image->size());
    *tmpPixmap = QPixmap::fromImage(*image);
    tmpPixmap->setDevicePixelRatio(qApp->devicePixelRatio());

    QString label;
    if(tmpPixmap->width() == 0) {
        label = QS("error");
    } else  {
        // put info into Thumbnail object
        label = image->text(QS("originalWidth")) +
                QS("x") +
                image->text(QS("originalHeight")) +
                image->text(QS("label"));
    }
    QSharedPointer<QPixmap> pixmapPtr(tmpPixmap);
    QSharedPointer<Thumbnail> thumbnail(new Thumbnail(imgInfo.fileName(), label, size, pixmapPtr));
    return thumbnail;
}

ThumbnailerRunnable::~ThumbnailerRunnable() {
}

std::pair<QImage*, QSize> ThumbnailerRunnable::createThumbnail(QString const &path, const char *format, int size, bool squared) {
    QImageReader *reader = new QImageReader(path, format);
    Qt::AspectRatioMode ARMode = squared?
                (Qt::KeepAspectRatioByExpanding):(Qt::KeepAspectRatio);
    QImage *result = nullptr;
    QSize originalSize;
    bool indexed = (reader->imageFormat() == QImage::Format_Indexed8);
    bool manualResize = indexed || !reader->supportsOption(QImageIOHandler::Size);
    if(!manualResize) { // resize during read via QImageReader (faster)
        QSize scaledSize = reader->size().scaled(size, size, ARMode);
        reader->setScaledSize(scaledSize);
        if(squared) {
            QRect clip(0, 0, size, size);
            QRect scaledRect(QPoint(0,0), scaledSize);
            clip.moveCenter(scaledRect.center());
            reader->setScaledClipRect(clip);
        }
        originalSize = reader->size();
        result = new QImage();
        if(!reader->read(result)) {
            // If read() returns false there's no guarantee that size conversion worked properly.
            // So we fallback to manual.
            // Se far I've seen this happen only on some weird (corrupted?) jpeg saved from camera
            manualResize = true;
            delete result;
            result = nullptr;
            // Force reset reader because it is really finicky
            // and can fail on the second read attempt (yeah wtf)
            reader->setFileName(QS(""));
            delete reader;
            reader = new QImageReader(path, format);
        }
    }
    if(manualResize) { // manual resize & crop. slower but should just work
        QImage *fullSize = new QImage();
        reader->read(fullSize);
        if(indexed) {
            auto newFmt = QImage::Format_RGB32;
            if(fullSize->hasAlphaChannel())
                newFmt = QImage::Format_ARGB32;
            auto tmp = new QImage(fullSize->convertToFormat(newFmt));
            delete fullSize;
            fullSize = tmp;
        }
        originalSize = fullSize->size();
        QSize scaledSize = fullSize->size().scaled(size, size, ARMode);
        if(squared) {
            QRect clip(0, 0, size, size);
            QRect scaledRect(QPoint(0,0), scaledSize);
            clip.moveCenter(scaledRect.center());
            QImage scaled = QImage(fullSize->scaled(scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            result = ImageLib::croppedRaw(&scaled, clip);
        } else {
            result = new QImage(fullSize->scaled(scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
        }
        delete fullSize;
    }
    // force reader to close file so it can be deleted later
    reader->setFileName(QS(""));
    delete reader;
    return std::make_pair(result, originalSize);
}

std::pair<QImage*, QSize> ThumbnailerRunnable::createVideoThumbnail(QString const &path, int size, bool squared) {
    QFileInfo fi(path);
    QImageReader reader;
    QString tmpFilePath = settings->tmpDir() + fi.fileName() + QS(".png");
    QString tmpFilePathEsc = tmpFilePath;
    tmpFilePathEsc.replace(QS("%"), QS("%%"));
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(settings->mpvBinary(),
                  QStringList() << QS("--start=30%")
                                << QS("--frames=1")
                                << QS("--aid=no")
                                << QS("--sid=no")
                                << QS("--no-config")
                                << QS("--load-scripts=no")
                                << QS("--no-terminal")
                                << QS("--o=") + tmpFilePathEsc
                                << path
                  );
    process.waitForFinished(8000);
    process.close();

    reader.setFileName(tmpFilePath);
    reader.setFormat("png");
    Qt::AspectRatioMode ARMode = squared?
                (Qt::KeepAspectRatioByExpanding):(Qt::KeepAspectRatio);
    QImage *result = nullptr;

    // scale & crop
    QSize scaledSize = reader.size().scaled(size, size, ARMode);
    reader.setScaledSize(scaledSize);
    if(squared) {
        QRect clip(0, 0, size, size);
        QRect scaledRect(QPoint(0,0), scaledSize);
        clip.moveCenter(scaledRect.center());
        reader.setScaledClipRect(clip);
    }
    QSize originalSize = reader.size();
    result = new QImage(reader.read());

    // force reader to close file so it can be deleted later
    reader.setFileName(QS(""));

    // remove temporary file
    QFile tmpFile(tmpFilePath);
    tmpFile.remove();

    return std::make_pair(result, originalSize);
}

#include "DocumentInfo.h"
#include "Platform.h"

DocumentInfo::DocumentInfo(QString const &path)
    : mOrientation(0),
      mDocumentType(DocumentType::NONE),
      exifLoaded(false)
{
    fileInfo.setFile(path);
    if (!fileInfo.isFile()) {
        qDebug() << u"FileInfo: cannot open:" << path;
        return;
    }
    detectFormat();
}

// ##############################################################
// ####################### PUBLIC METHODS #######################
// ##############################################################

QString DocumentInfo::directoryPath() const
{
    return fileInfo.absolutePath();
}

QString DocumentInfo::filePath() const
{
    return fileInfo.absoluteFilePath();
}

QString DocumentInfo::fileName() const
{
    return fileInfo.fileName();
}

QString DocumentInfo::baseName() const
{
    return fileInfo.baseName();
}

// bytes
qint64 DocumentInfo::fileSize() const
{
    return fileInfo.size();
}

DocumentType DocumentInfo::type() const
{
    return mDocumentType;
}

QMimeType DocumentInfo::mimeType() const
{
    return mMimeType;
}

QByteArray DocumentInfo::format() const
{
    return mFormat;
}

QDateTime DocumentInfo::lastModified() const
{
    return fileInfo.lastModified();
}

// For cases like orientation / even mimetype change we just reload
// Image from scratch, so don`t bother handling it here
void DocumentInfo::refresh()
{
    fileInfo.refresh();
}

int DocumentInfo::exifOrientation() const
{
    return mOrientation;
}

// ##############################################################
// ####################### PRIVATE METHODS ######################
// ##############################################################
void DocumentInfo::detectFormat()
{
    if (mDocumentType != DocumentType::NONE)
        return;

    QMimeDatabase mimeDb;
    mMimeType         = mimeDb.mimeTypeForFile(fileInfo.filePath(), QMimeDatabase::MatchContent);
    auto mimeName     = mMimeType.name();
    auto mimeNameUTF8 = mimeName.toUtf8();
    auto suffix       = fileInfo.suffix().toLower().toUtf8();

    if (mimeName == u"image/jpeg"_sv) {
        mFormat = "jpg"_ba;
        mDocumentType = DocumentType::STATIC;
    } else if (mimeName == u"image/png"_sv) {
        if (QImageReader::supportedImageFormats().contains("apng"_ba) && detectAPNG()) {
            mFormat = "apng"_ba;
            mDocumentType = DocumentType::ANIMATED;
        } else {
            mFormat = "png"_ba;
            mDocumentType = DocumentType::STATIC;
        }
    } else if (mimeName == u"image/gif"_sv) {
        mFormat = "gif"_ba;
        mDocumentType = DocumentType::ANIMATED;
    } else if (mimeName == u"image/webp"_sv || (mimeName == u"audio/x-riff"_sv && suffix == "webp"_ba)) {
        mFormat = "webp";
        mDocumentType = detectAnimatedWebP() ? DocumentType::ANIMATED : DocumentType::STATIC;
    } else if (mimeName == u"image/jxl"_sv) {
        mFormat = "jxl"_ba;
        mDocumentType = detectAnimatedJxl() ? DocumentType::ANIMATED : DocumentType::STATIC;
        if (mDocumentType == DocumentType::ANIMATED && !settings->jxlAnimation()) {
            mDocumentType = DocumentType::NONE;
            qDebug() << u"animated jxl is off; skipping file";
        }
    } else if (mimeName == u"image/avif"_sv) {
        mFormat = "avif"_ba;
        mDocumentType = detectAnimatedAvif() ? DocumentType::ANIMATED : DocumentType::STATIC;
    } else if (mimeName == u"image/bmp"_sv) {
        mFormat = "bmp"_ba;
        mDocumentType = DocumentType::STATIC;
    } else if (settings->videoPlayback() && settings->videoFormats().contains(mimeNameUTF8)) {
        mDocumentType = DocumentType::VIDEO;
        mFormat = settings->videoFormats().value(mimeNameUTF8);
    } else {
        // just try to open via suffix if all of the above fails
        mFormat = suffix;
        if (mFormat.compare("jfif"_ba, Qt::CaseInsensitive) == 0)
            mFormat = "jpg"_ba;
        if (settings->videoPlayback() && settings->videoFormats().values().contains(suffix))
            mDocumentType = DocumentType::VIDEO;
        else
            mDocumentType = DocumentType::STATIC;
    }

    loadExifOrientation();
}

// dumb apng detector
inline bool DocumentInfo::detectAPNG() const
{
    QFile f(fileInfo.filePath());

    if (f.open(QFile::ReadOnly)) {
        constexpr int len = 120;
        QDataStream   in(&f);
        QByteArray    qbuf("\0", len);
        if (in.readRawData(qbuf.data(), len) > 0)
            return qbuf.contains("acTL");
    }

    return false;
}

bool DocumentInfo::detectAnimatedWebP() const
{
    QFile f(fileInfo.filePath());
    bool  result = false;
    if (f.open(QFile::ReadOnly)) {
        QDataStream in(&f);
        in.skipRawData(12);
        char buf[5];
        buf[4] = '\0';
        in.readRawData(buf, 4);
        if (strcmp(buf, "VP8X") == 0) {
            in.skipRawData(4);
            char flags;
            in.readRawData(&flags, 1);
            if (flags & (1 << 1))
                result = true;
        }
    }
    return result;
}

// TODO avoid creating multiple QImageReader instances
bool DocumentInfo::detectAnimatedJxl() const
{
    QImageReader r(fileInfo.filePath(), "jxl");
    return r.supportsAnimation();
}

bool DocumentInfo::detectAnimatedAvif() const
{
    QFile f(fileInfo.filePath());
    bool  result = false;
    if (f.open(QFile::ReadOnly)) {
        QDataStream in(&f);
        in.skipRawData(4); // skip box size
        char buf[9];
        buf[8] = '\0';
        in.readRawData(buf, 8);
        if (strcmp(buf, "ftypavis") == 0)
            result = true;
    }
    return result;
}

void DocumentInfo::loadExifTags()
{
    if (exifLoaded)
        return;
    exifLoaded = true;
    exifTags.clear();

#ifdef USE_EXIV2
    try {
# if defined Q_OS_WINDOWS
        auto u8path = R"(\\?\)"s + QDir::toNativeSeparators(fileInfo.absoluteFilePath()).toStdString();
# else
        auto u8path = fileInfo.absoluteFilePath().toStdString();
# endif
        auto image = Exiv2::ImageFactory::open(u8path);
        assert(image != nullptr);

        image->readMetadata();
        Exiv2::ExifData &exifData = image->exifData();
        if (exifData.empty())
            return;

        auto make            = Exiv2::ExifKey("Exif.Image.Make"s);
        auto model           = Exiv2::ExifKey("Exif.Image.Model"s);
        auto dateTime        = Exiv2::ExifKey("Exif.Image.DateTime"s);
        auto exposureTime    = Exiv2::ExifKey("Exif.Photo.ExposureTime"s);
        auto fnumber         = Exiv2::ExifKey("Exif.Photo.FNumber"s);
        auto isoSpeedRatings = Exiv2::ExifKey("Exif.Photo.ISOSpeedRatings"s);
        auto flash           = Exiv2::ExifKey("Exif.Photo.Flash"s);
        auto focalLength     = Exiv2::ExifKey("Exif.Photo.FocalLength"s);
        auto userComment     = Exiv2::ExifKey("Exif.Photo.UserComment"s);

        Exiv2::ExifData::const_iterator it;

        it = exifData.findKey(make);
        if (it != exifData.end() /* && it->count() */)
            exifTags.insert(QObject::tr("Make"), QString::fromStdString(it->toString()));

        it = exifData.findKey(model);
        if (it != exifData.end())
            exifTags.insert(QObject::tr("Model"), QString::fromStdString(it->toString()));

        it = exifData.findKey(dateTime);
        if (it != exifData.end())
            exifTags.insert(QObject::tr("Date/Time"), QString::fromStdString(it->toString()));

        it = exifData.findKey(exposureTime);
        if (it != exifData.end()) {
            Exiv2::Rational r = it->toRational();
            if (r.first < r.second) {
                qreal exp = round(static_cast<qreal>(r.second) / r.first);
                exifTags.insert(QObject::tr("ExposureTime"), u"1/" + QString::number(exp) + QObject::tr(" sec"));
            } else {
                qreal exp = round(static_cast<qreal>(r.first) / r.second);
                exifTags.insert(QObject::tr("ExposureTime"), QString::number(exp) + QObject::tr(" sec"));
            }
        }

        it = exifData.findKey(fnumber);
        if (it != exifData.end()) {
            auto r  = it->toRational();
            auto fn = static_cast<qreal>(r.first) / static_cast<qreal>(r.second);
            exifTags.insert(QObject::tr("F Number"), u"f/" + QString::number(fn, 'g', 3));
        }

        it = exifData.findKey(isoSpeedRatings);
        if (it != exifData.end())
            exifTags.insert(QObject::tr("ISO Speed ratings"), QString::fromStdString(it->toString()));

        it = exifData.findKey(flash);
        if (it != exifData.end())
            exifTags.insert(QObject::tr("Flash"), QString::fromStdString(it->toString()));

        it = exifData.findKey(focalLength);
        if (it != exifData.end()) {
            auto r  = it->toRational();
            auto fn = static_cast<qreal>(r.first) / r.second;
            exifTags.insert(QObject::tr("Focal Length"), QString::number(fn, 'g', 3) + QObject::tr(" mm"));
        }

        it = exifData.findKey(userComment);
        if (it != exifData.end()) {
            // This frequently throws.
            try {
                // crop out 'charset=ascii' etc"
                if (it->size() > 0) {
                    auto stdStr = it->toString();
                    if (!stdStr.empty()) {
                        auto comment = QString::fromStdString(stdStr);
                        if (comment.startsWith(u"charset="_sv))
                            comment.remove(0, comment.indexOf(u' ') + 1);
                        exifTags.insert(QObject::tr("UserComment"), comment);
                    }
                }
            } catch (Exiv2::Error &e) {
                qDebug() << u"Caught Exiv2 exception, attempting to ignore:" << e.what();
            }
        }
    }

// this should work with both 0.28 and <0.28
# if !EXIV2_TEST_VERSION(0, 28, 0)
#  ifdef _WIN32
    catch (Exiv2::BasicError<wchar_t>& e) {
        qDebug() << u"Caught Exiv2::BasicError exception:" << e.what();
    }
#  else
    catch (Exiv2::BasicError<char>& e) {
        qDebug() << u"Caught Exiv2::BasicError exception:" << e.what();
    }
#  endif
# endif
    catch (Exiv2::Error& e) {
        qDebug() << u"Caught Exiv2 exception:" << e.what();
    }
#endif
}

QMap<QString, QString> DocumentInfo::getExifTags()
{
    if (!exifLoaded)
        loadExifTags();
    return exifTags;
}

void DocumentInfo::loadExifOrientation()
{
    if (mDocumentType == DocumentType::VIDEO || mDocumentType == DocumentType::NONE)
        return;
    auto reader = std::make_unique<QImageReader>(filePath(), mFormat);
    if (reader->canRead())
        mOrientation = static_cast<int>(reader->transformation());
}

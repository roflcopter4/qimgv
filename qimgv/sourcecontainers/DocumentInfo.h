#pragma once

#include <QString>
#include <QSize>
#include <QUrl>
#include <QMimeDatabase>
#include <QDebug>
#include <QFileInfo>
#include <QDateTime>
#include <cmath>
#include <cstring>
#include "utils/Stuff.h"
#include "Settings.h"

#ifdef USE_EXIV2

#include <exiv2/exiv2.hpp>
#include <iostream>
#include <iomanip>
#include <cassert>

#endif

#include <QImageReader>


enum class DocumentType : uint8_t {
    NONE,
    STATIC,
    ANIMATED,
    VIDEO,
};

class DocumentInfo
{
  public:
    explicit DocumentInfo(QString const &path);
    ~DocumentInfo() = default;

    ND QString      directoryPath() const;
    ND QString      filePath() const;
    ND QString      fileName() const;
    ND QString      baseName() const;
    ND qint64       fileSize() const;
    ND DocumentType type() const;
    ND QMimeType    mimeType() const;

    // file extension (guessed from mime-type)
    ND QByteArray format() const;
    ND QDateTime  lastModified() const;
    ND int        exifOrientation() const;

    void refresh();
    void loadExifTags();

    QMap<QString, QString> getExifTags();

    DocumentInfo(DocumentInfo const &)            = default;
    DocumentInfo(DocumentInfo &&)                 = default;
    DocumentInfo &operator=(DocumentInfo const &) = default;
    DocumentInfo &operator=(DocumentInfo &&)      = default;

  private:
    QFileInfo    fileInfo;
    QByteArray   mFormat;
    int          mOrientation;
    DocumentType mDocumentType;
    bool         exifLoaded;

    // guesses file type from its contents
    // and sets extension
    void detectFormat();
    void loadExifOrientation();
    bool detectAPNG() const;
    bool detectAnimatedWebP() const;
    bool detectAnimatedJxl() const;
    bool detectAnimatedAvif() const;

    QMap<QString, QString> exifTags;
    QMimeType              mMimeType;
};

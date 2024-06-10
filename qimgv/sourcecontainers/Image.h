#pragma once

#include "sourcecontainers/DocumentInfo.h"
#include "utils/ImageLib.h"
#include "utils/Stuff.h"
#include <QDebug>
#include <QPixmap>
#include <QString>
#include <memory>

class Image
{
  public:
    explicit Image(QString const &);
    explicit Image(std::unique_ptr<DocumentInfo>);
    virtual ~Image() = default;

    ND virtual auto  getPixmap() -> std::unique_ptr<QPixmap>    = 0;
    ND virtual auto  getImage() -> QSharedPointer<QImage const> = 0;
    ND virtual int   height() const                             = 0;
    ND virtual int   width() const                              = 0;
    ND virtual QSize size() const                               = 0;

    virtual bool save()                 = 0;
    virtual bool save(QString destPath) = 0;

    ND auto baseName() const -> QString;
    ND auto fileName() const -> QString;
    ND auto filePath() const -> QString;
    ND auto fileSize() const -> qint64;
    ND auto getExifTags() const -> QMap<QString, QString>;
    ND auto lastModified() const -> QDateTime;
    ND auto type() const -> DocumentType;
    ND bool isEdited() const;
    ND bool isLoaded() const;

  protected:
    virtual void load() = 0;

    std::unique_ptr<DocumentInfo> mDocInfo;
    bool    mLoaded;
    bool    mEdited;
    QString mPath;
    QSize   resolution;
};

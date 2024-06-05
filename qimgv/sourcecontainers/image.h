#pragma once

#include "sourcecontainers/documentinfo.h"
#include "utils/imagelib.h"
#include "utils/stuff.h"
#include <QDebug>
#include <QPixmap>
#include <QString>
#include <memory>

#include "Common.h"

class Image
{
  public:
    explicit Image(QString const &);
    explicit Image(std::unique_ptr<DocumentInfo>);
    virtual ~Image() = default;

    virtual std::unique_ptr<QPixmap>      getPixmap() = 0;
    virtual QSharedPointer<QImage const> getImage()  = 0;

    virtual int   height()               = 0;
    virtual int   width()                = 0;
    virtual QSize size()                 = 0;
    virtual bool  save()                 = 0;
    virtual bool  save(QString destPath) = 0;

    ND DocumentType type() const;
    ND QString      filePath() const;
    ND bool         isLoaded() const;
    ND QString      fileName() const;
    ND QString      baseName() const;
    ND bool         isEdited() const;
    ND qint64       fileSize() const;
    ND QDateTime    lastModified() const;

    ND QMap<QString, QString> getExifTags() const;

  protected:
    virtual void load() = 0;

    std::unique_ptr<DocumentInfo> mDocInfo;
    bool    mLoaded;
    bool    mEdited;
    QString mPath;
    QSize   resolution;
};

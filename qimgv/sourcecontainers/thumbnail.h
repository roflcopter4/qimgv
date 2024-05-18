#pragma once

#include <QString>
#include <QPixmap>
#include <memory>

#include "Common.h"

class Thumbnail
{
  public:
    Thumbnail(QString name, QString info, int size, std::shared_ptr<QPixmap> const &pixmap);

    ND QString const &name() const &;
    ND QString const &info() const &;

    ND int  size() const;
    ND bool hasAlphaChannel() const;

    ND std::shared_ptr<QPixmap> pixmap();

  private:
    std::shared_ptr<QPixmap> mPixmap;

    QString mName;
    QString mInfo;
    int     mSize;
    bool    mHasAlphaChannel;
};

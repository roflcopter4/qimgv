#pragma once

#include <QString>
#include <QPixmap>
#include <memory>
#include <QSharedPointer>

class Thumbnail
{
  public:
    Thumbnail(QString name, QString info, int size, QSharedPointer<QPixmap> pixmap);

    ND QString const &name() const &;
    ND QString const &info() const &;

    ND int  size() const;
    ND bool hasAlphaChannel() const;

    ND QSharedPointer<QPixmap> pixmap();
    ND QSharedPointer<QPixmap> const &pixmap() const;

  private:
    QSharedPointer<QPixmap> mPixmap;

    QString mName;
    QString mInfo;
    int     mSize;
    bool    mHasAlphaChannel;
};

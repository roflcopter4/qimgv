#include "thumbnail.h"

Thumbnail::Thumbnail(QString name, QString info, int size, QSharedPointer<QPixmap> pixmap)
    : mPixmap(pixmap),
      mName(std::move(name)),
      mInfo(std::move(info)),
      mSize(size)
{
    if (mPixmap)
        mHasAlphaChannel = mPixmap->hasAlphaChannel();
}

QString const &Thumbnail::name() const &
{
    return mName;
}

QString const &Thumbnail::info() const &
{
    return mInfo;
}

int Thumbnail::size() const
{
    return mSize;
}

bool Thumbnail::hasAlphaChannel() const
{
    return mHasAlphaChannel;
}

QSharedPointer<QPixmap> Thumbnail::pixmap()
{
    return mPixmap;
}

QSharedPointer<QPixmap> const &Thumbnail::pixmap() const
{
    return mPixmap;
}

#include "thumbnail.h"

Thumbnail::Thumbnail(QString name, QString info, int size, std::shared_ptr<QPixmap> const &pixmap)
    : mPixmap(pixmap),
      mName(std::move(name)),
      mInfo(std::move(info)),
      mSize(size)
{
    if (pixmap)
        mHasAlphaChannel = pixmap->hasAlphaChannel();
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

std::shared_ptr<QPixmap> Thumbnail::pixmap()
{
    return mPixmap;
}

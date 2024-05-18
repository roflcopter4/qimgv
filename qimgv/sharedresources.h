#pragma once

#include <QDebug>
#include <QPixmap>

enum class ShrIcon {
    Error,
    Loading,
};

class SharedResources
{
  public:
    SharedResources();
    ~SharedResources();
    static SharedResources *getInstance();

    QPixmap *getPixmap(ShrIcon icon, qreal dpr);

  private:
    QPixmap *mLoadingIcon72      = nullptr;
    QPixmap *mLoadingErrorIcon72 = nullptr;
};

extern SharedResources *shrRes;

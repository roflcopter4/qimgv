#pragma once

#include "image.h"
#include <QMovie>
#include <QTimer>

#include "Common.h"

class ImageAnimated : public Image
{
  public:
    explicit ImageAnimated(QString const &path);
    explicit ImageAnimated(std::unique_ptr<DocumentInfo> info);
    ~ImageAnimated() override = default;

    ND auto getMovie() -> QSharedPointer<QMovie>;
    ND auto getPixmap() -> std::unique_ptr<QPixmap> override;
    ND auto getImage() -> QSharedPointer<QImage const> override;
    ND int   height() const override;
    ND int   width() const override;
    ND QSize size() const override;
    ND int frameCount() const;

  public Q_SLOTS:
    bool save() override;
    bool save(QString destPath) override;

  Q_SIGNALS:
    void frameChanged(QPixmap *);

  private:
    int   mFrameCount;
    QSize mSize;
    QSharedPointer<QMovie> movie;

    void load() override;
    void loadMovie();
};

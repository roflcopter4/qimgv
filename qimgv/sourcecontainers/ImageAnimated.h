#pragma once

#include "Image.h"
#include <QMovie>
#include <QTimer>

class ImageAnimated : public Image
{
  public:
    explicit ImageAnimated(QString const &path);
    explicit ImageAnimated(std::unique_ptr<DocumentInfo> info);
    ~ImageAnimated() override = default;
    DELETE_COPY_MOVE_ROUTINES(ImageAnimated);

    ND auto getMovie() -> QSharedPointer<QMovie>;
    ND auto getPixmap() -> std::unique_ptr<QPixmap> override;
    ND auto getImage() -> QSharedPointer<QImage const> override;
    ND int   height() const override;
    ND int   width() const override;
    ND QSize size() const override;
    ND int frameCount() const;

  Q_SIGNALS:
    void frameChanged(QPixmap *);

  public Q_SLOTS:
    bool save() override;
    bool save(QString destPath) override;

  private:
    void load() override;
    void loadMovie();

    QSharedPointer<QMovie> movie;
    QSize mSize{0, 0};
    int   mFrameCount;
};

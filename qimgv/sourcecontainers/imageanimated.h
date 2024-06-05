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

    std::unique_ptr<QPixmap>      getPixmap() override;
    QSharedPointer<QImage const> getImage() override;
    QSharedPointer<QMovie>       getMovie();

    int   height() override;
    int   width() override;
    QSize size() override;

    bool isEditable();
    bool isEdited();

    ND int frameCount() const;

  public slots:
    bool save() override;
    bool save(QString destPath) override;

  signals:
    void frameChanged(QPixmap *);

  private:
    QSize mSize;
    int   mFrameCount;
    QSharedPointer<QMovie> movie;

    void load() override;
    void loadMovie();
};

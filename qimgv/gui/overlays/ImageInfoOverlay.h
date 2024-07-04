#pragma once

#include "gui/customWidgets/OverlayWidget.h"
#include "gui/customWidgets/EntryInfoItem.h"
#include <QWheelEvent>

namespace Ui {
class ImageInfoOverlay;
}

class ImageInfoOverlay : public OverlayWidget
{
    Q_OBJECT

  public:
    explicit ImageInfoOverlay(FloatingWidgetContainer *parent = nullptr);
    ~ImageInfoOverlay() override;
    DELETE_COPY_MOVE_ROUTINES(ImageInfoOverlay);

    void setExifInfo(QMap<QString, QString> const &);

  public Q_SLOTS:
    void show();

  protected:
    void wheelEvent(QWheelEvent *event) override;

  private:
    Ui::ImageInfoOverlay  *ui;
    QList<EntryInfoItem *> entries;
    QLabel                 entryStub;
};

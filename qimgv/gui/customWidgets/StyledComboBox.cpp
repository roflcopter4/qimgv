#include "StyledComboBox.h"

StyledComboBox::StyledComboBox(QWidget *parent)
    : QComboBox(parent),
      dpr(devicePixelRatioF()),
      pixmapDrawScale(0.0),
      hiResPixmap(false)
{
    connect(settings, &Settings::settingsChanged, this, &StyledComboBox::onSettingsChanged);
}

StyledComboBox::~StyledComboBox() = default;

void StyledComboBox::onSettingsChanged()
{
    ImageLib::recolor(downArrow, settings->colorScheme().icons);
}

void StyledComboBox::setIconPath(QString path)
{
    if (dpr >= 1.0 + 0.001) {
        path.replace(u'.', u"@2x."_s);
        hiResPixmap = true;
        downArrow.load(path);
        pixmapDrawScale = dpr >= 2.0 - 0.001 ? dpr : 2.0;
        downArrow.setDevicePixelRatio(pixmapDrawScale);
    } else {
        hiResPixmap = false;
        downArrow.load(path);
        pixmapDrawScale = dpr;
    }
    ImageLib::recolor(downArrow, settings->colorScheme().icons);
    update();
}

void StyledComboBox::paintEvent(QPaintEvent *e)
{
    QComboBox::paintEvent(e);
    QPainter p(this);
    QPointF  pos;

    if (hiResPixmap) {
        pos = QPointF(width() - 8 - downArrow.width() / pixmapDrawScale,
                      static_cast<qreal>(height()) / 2.0 - downArrow.height() / (2.0 * pixmapDrawScale));
    } else {
        pos = QPointF(width() - downArrow.width() - 8, static_cast<qreal>(height() - downArrow.height()) / 2.0);
    }
    p.drawPixmap(pos, downArrow);
}

void StyledComboBox::keyPressEvent(QKeyEvent *event)
{
    event->ignore();
}

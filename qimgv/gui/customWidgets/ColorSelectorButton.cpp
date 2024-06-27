#include "ColorSelectorButton.h"

ColorSelectorButton::ColorSelectorButton(QWidget *parent)
    : ClickableLabel(parent)
{
    connect(this, &ColorSelectorButton::clicked, this, &ColorSelectorButton::showColorSelector);
}

void ColorSelectorButton::setColor(QColor const &newColor)
{
    mColor = newColor;
    update();
}

void ColorSelectorButton::setDescription(QString const &text)
{
    mDescription = text;
}

QColor ColorSelectorButton::color() const
{
    return mColor;
}

void ColorSelectorButton::showColorSelector()
{
    QColor newColor = QColorDialog::getColor(mColor, this, mDescription);
    if (newColor.isValid()) {
        mColor = newColor;
        update();
    }
}

void ColorSelectorButton::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    if (!isEnabled())
        p.setOpacity(0.5f);
    p.setPen(QColor(40, 40, 40));
    p.drawRect(QRectF(0.5f, 0.5f, width() - 1.0f, height() - 1.0f));
    p.fillRect(rect().adjusted(2, 2, -2, -2), mColor);
}
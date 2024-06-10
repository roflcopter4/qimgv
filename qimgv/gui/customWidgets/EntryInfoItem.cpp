#include "EntryInfoItem.h"

EntryInfoItem::EntryInfoItem(QWidget *parent)
    : QWidget(parent),
      layout(new QHBoxLayout(this))
{
    layout->setContentsMargins(9, 0, 9, 0);
    layout->setSpacing(0);
    layout->addWidget(&nameLabel);
    layout->addWidget(&valueLabel);
    setLayout(layout);

    nameLabel.setFixedSize(120, 30);
    valueLabel.setFixedSize(142, 30);

    // add some padding for easier text selection
    valueLabel.setContentsMargins(3, 0, 0, 0);
    valueLabel.setTextInteractionFlags(Qt::TextSelectableByMouse);
    valueLabel.setCursor(Qt::IBeamCursor);
}

void EntryInfoItem::setInfo(QString newName, QString newValue)
{
    name  = std::move(newName);
    value = std::move(newValue);
    nameLabel.setText(name);
    valueLabel.setText(value);
}

void EntryInfoItem::paintEvent([[maybe_unused]] QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

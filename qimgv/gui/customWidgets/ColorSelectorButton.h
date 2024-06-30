#pragma once

#include "gui/customWidgets/ClickableLabel.h"
#include <QColorDialog>
#include <QPainter>

class ColorSelectorButton : public ClickableLabel
{
    Q_OBJECT

  public:
    explicit ColorSelectorButton(QWidget *parent = nullptr);

    void setColor(QColor const &newColor);
    void setDescription(QString const &text);

    ND QColor color() const;

  protected:
    void paintEvent(QPaintEvent *e) override;

  private Q_SLOTS:
    void showColorSelector();

  private:
    QColor  mColor;
    QString mDescription;
};

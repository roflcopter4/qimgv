#ifndef COLORSELECTORBUTTON_H
#define COLORSELECTORBUTTON_H

#include <QPainter>
#include <QColorDialog>
#include "gui/customwidgets/clickablelabel.h"

class ColorSelectorButton : public ClickableLabel {
    Q_OBJECT
public:
    explicit ColorSelectorButton(QWidget *parent = nullptr);

    void   setColor(QColor const &newColor);
    QColor color() const;
    void   setDescription(QString const &text);

protected:
    void paintEvent(QPaintEvent *e) override;

private slots:
    void showColorSelector();

private:
    QColor mColor;
    QString mDescription;
};

#endif // COLORSELECTORBUTTON_H

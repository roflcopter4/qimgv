#ifndef STYLEDCOMBOBOX_H
#define STYLEDCOMBOBOX_H

#include <QComboBox>
#include <QPainter>
#include <QKeyEvent>
#include "Settings.h"
#include "utils/imagelib.h"

class StyledComboBox : public QComboBox
{
  public:
    explicit StyledComboBox(QWidget *parent = nullptr);
    void setIconPath(QString path);

  protected:
    void paintEvent(QPaintEvent *e) override;
    void keyPressEvent(QKeyEvent *event) override;

  private:
    bool    hiResPixmap;
    QPixmap downArrow;
    qreal   dpr;
    qreal   pixmapDrawScale;
};

#endif // STYLEDCOMBOBOX_H

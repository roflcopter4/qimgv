#pragma once

#include "Settings.h"
#include "utils/ImageLib.h"
#include <QComboBox>
#include <QKeyEvent>
#include <QPainter>

class StyledComboBox : public QComboBox
{
  public:
    explicit StyledComboBox(QWidget *parent = nullptr);
    void setIconPath(QString path);

  protected:
    void paintEvent(QPaintEvent *e) override;
    void keyPressEvent(QKeyEvent *event) override;

  private Q_SLOTS:
    void onSettingsChanged();

  private:
    bool    hiResPixmap;
    QPixmap downArrow;
    qreal   dpr;
    qreal   pixmapDrawScale;
};

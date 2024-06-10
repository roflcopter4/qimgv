#pragma once

#include "ShortcutBuilder.h"
#include "utils/Actions.h"
#include <QPushButton>

class KeySequenceEdit : public QPushButton
{
    Q_OBJECT

  public:
    explicit KeySequenceEdit(QWidget *parent);

    QString  sequence();
    void     setText(const QString &text);

  protected:
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;

    void mouseReleaseEvent(QMouseEvent *e) override;
    bool focusNextPrevChild(bool) override;

  private:
    void    processEvent(QEvent *e);
    QString mSequence;

  Q_SIGNALS:
    void edited();
};

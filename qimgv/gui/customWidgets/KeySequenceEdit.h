#pragma once

#include "ShortcutBuilder.h"
#include "utils/Actions.h"
#include <QPushButton>

class KeySequenceEdit : public QPushButton
{
    Q_OBJECT

  public:
    explicit KeySequenceEdit(QWidget *parent);

    void setText(QString const &text);

    ND QString sequence() const;

  protected:
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;

    void mouseReleaseEvent(QMouseEvent *e) override;
    bool focusNextPrevChild(bool) override;

  Q_SIGNALS:
    void edited();

  private:
    QString mSequence;

    void processEvent(QEvent *e);
};

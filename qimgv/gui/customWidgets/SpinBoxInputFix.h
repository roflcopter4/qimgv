// this spin box ignores all keyboard input except numbers, up/down keys

#pragma once

#include "components/actionManager/ActionManager.h"
#include <QDebug>
#include <QKeyEvent>
#include <QSpinBox>

class SpinBoxInputFix : public QSpinBox
{
    Q_OBJECT

  public:
    explicit SpinBoxInputFix(QWidget *parent);

  protected:
    void keyPressEvent(QKeyEvent *event) override;

  private:
    static constexpr auto allowedKeys = std::array{
        Qt::Key::Key_0,         Qt::Key::Key_1,     Qt::Key::Key_2,      Qt::Key::Key_3,    Qt::Key::Key_4,
        Qt::Key::Key_5,         Qt::Key::Key_6,     Qt::Key::Key_7,      Qt::Key::Key_8,    Qt::Key::Key_9,
        Qt::Key::Key_Backspace, Qt::Key::Key_Enter, Qt::Key::Key_Delete, Qt::Key::Key_Home, Qt::Key::Key_End,
        Qt::Key::Key_Left,      Qt::Key::Key_Up,    Qt::Key::Key_Right,  Qt::Key::Key_Down, Qt::Key::Key_PageUp,
        Qt::Key::Key_PageDown,
    };
};

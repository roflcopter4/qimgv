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
    SpinBoxInputFix(QWidget *parent = nullptr);

private:
    QStringList allowedKeys;

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

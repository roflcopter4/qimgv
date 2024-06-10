#pragma once

#include "components/actionManager/ActionManager.h"
#include "components/scriptManager/ScriptManager.h"
#include "ShortcutBuilder.h"
#include "utils/Actions.h"
#include <QComboBox>
#include <QDialog>
#include <QRadioButton>

// TODO: separate gui from components
// OR move script & action stuff to project root?

namespace Ui {
class ShortcutCreatorDialog;
}

class ShortcutCreatorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ShortcutCreatorDialog(QWidget *parent = nullptr);
    ~ShortcutCreatorDialog() override;
    QString selectedAction();
    QString selectedShortcut();
    void setAction(QString);
    void setShortcut(QString);

private slots:
    void onShortcutEdited();

private:
    Ui::ShortcutCreatorDialog *ui;
    QList<QString> actionList, scriptList;
};

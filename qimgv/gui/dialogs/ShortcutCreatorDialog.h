#pragma once

#include "ShortcutBuilder.h"
#include "components/actionManager/ActionManager.h"
#include "components/scriptManager/ScriptManager.h"
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

    ND QString selectedAction() const;
    ND QString selectedShortcut() const;

    void setAction(QString action);
    void setShortcut(QString const &shortcut);

  private Q_SLOTS:
    void onShortcutEdited();

  private:
    Ui::ShortcutCreatorDialog *ui;

    QList<QString> actionList;
    QList<QString> scriptList;
};

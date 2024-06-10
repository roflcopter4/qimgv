#pragma once

#include "components/scriptManager/ScriptManager.h"
#include "utils/Script.h"
#include <QCheckBox>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

namespace Ui {
class ScriptEditorDialog;
}

class ScriptEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ScriptEditorDialog(QWidget *parent = nullptr);
    explicit ScriptEditorDialog(QString name, Script const &script, QWidget *parent = nullptr);
    ~ScriptEditorDialog() override;
    QString scriptName() const;
    Script  script() const;

private slots:
    void onNameChanged(QString const &name);

    void selectScriptPath();
private:
    Ui::ScriptEditorDialog *ui;
    bool editMode;
    QString editTarget;
};

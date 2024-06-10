#pragma once

#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include "components/scriptmanager/scriptmanager.h"
#include "utils/script.h"

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

#include "scripteditordialog.h"
#include "ui_scripteditordialog.h"

ScriptEditorDialog::ScriptEditorDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::ScriptEditorDialog),
      editMode(false)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("New application/script"));
    ui->keywordsLabel->setText(tr("Keywords:") + QSV(" %file%"));
    connect(ui->nameLineEdit, &QLineEdit::textChanged, this, &ScriptEditorDialog::onNameChanged);
    this->onNameChanged(ui->nameLineEdit->text());
}

ScriptEditorDialog::ScriptEditorDialog(QString name, Script const &script, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::ScriptEditorDialog),
      editMode(true)
{
    ui->setupUi(this);
    this->setWindowTitle(tr("Edit"));
    this->onNameChanged(ui->nameLineEdit->text());
    editTarget = std::move(name);
    connect(ui->nameLineEdit, &QLineEdit::textChanged, this, &ScriptEditorDialog::onNameChanged);
    ui->nameLineEdit->setText(editTarget);
    ui->pathLineEdit->setText(script.command);
    ui->blockingCheckBox->setChecked(script.blocking);
    this->onNameChanged(ui->nameLineEdit->text());
}

ScriptEditorDialog::~ScriptEditorDialog()
{
    delete ui;
}

QString ScriptEditorDialog::scriptName() const
{
    return ui->nameLineEdit->text();
}

Script ScriptEditorDialog::script() const
{
    return Script{ui->pathLineEdit->text(), ui->blockingCheckBox->isChecked()};
}

void ScriptEditorDialog::onNameChanged(QString const &name)
{
    if (name.isEmpty()) {
        ui->messageLabel->setText(tr("Enter script name"));
        ui->acceptButton->setEnabled(false);
        return;
    } else {
        ui->acceptButton->setEnabled(true);
    }

    QString okBtnText;
    ui->messageLabel->clear();

    if (editMode) {
        if (name != editTarget && scriptManager->scriptExists(name)) {
            ui->messageLabel->setText(tr("A script with this same name exists"));
            okBtnText = QS("Replace");
        } else {
            okBtnText = QS("Save");
        }
    } else if (scriptManager->scriptExists(name)) {
        ui->messageLabel->setText(tr("A script with this same name exists"));
        okBtnText = QS("Replace");
    } else {
        okBtnText = QS("Create");
    }
    ui->acceptButton->setText(okBtnText);
}

void ScriptEditorDialog::selectScriptPath()
{
    QString file;
#ifdef Q_OS_WIN32
    file = QFileDialog::getOpenFileName(this, tr("Select an executable/script"), {}, QS("Executable/script (*.exe *.bat)"));
#else
    file = QFileDialog::getOpenFileName(this, tr("Select a script file"), {}, QS("Shell script (*.sh)"));
#endif
    if (!file.isEmpty())
        ui->pathLineEdit->setText(u'"' + file + u'"' + QSV(" %file%"));
}

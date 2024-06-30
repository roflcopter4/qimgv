#include "ScriptEditorDialog.h"
#include "ui_ScriptEditorDialog.h"

ScriptEditorDialog::ScriptEditorDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::ScriptEditorDialog),
      editMode(false)
{
    ui->setupUi(this);
    setWindowTitle(tr("New application/script"));
    ui->keywordsLabel->setText(tr("Keywords:") + u" %file%");
    connect(ui->nameLineEdit, &QLineEdit::textChanged, this, &ScriptEditorDialog::onNameChanged);
    onNameChanged(ui->nameLineEdit->text());
}

ScriptEditorDialog::ScriptEditorDialog(QString name, Script const &script, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::ScriptEditorDialog),
      editMode(true)
{
    ui->setupUi(this);
    setWindowTitle(tr("Edit"));
    onNameChanged(ui->nameLineEdit->text());
    editTarget = std::move(name);
    connect(ui->nameLineEdit, &QLineEdit::textChanged, this, &ScriptEditorDialog::onNameChanged);
    ui->nameLineEdit->setText(editTarget);
    ui->pathLineEdit->setText(script.command);
    ui->blockingCheckBox->setChecked(script.blocking);
    onNameChanged(ui->nameLineEdit->text());
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
            okBtnText = u"Replace"_s;
        } else {
            okBtnText = u"Save"_s;
        }
    } else if (scriptManager->scriptExists(name)) {
        ui->messageLabel->setText(tr("A script with this same name exists"));
        okBtnText = u"Replace"_s;
    } else {
        okBtnText = u"Create"_s;
    }
    ui->acceptButton->setText(okBtnText);
}

void ScriptEditorDialog::selectScriptPath()
{
    QString file;
#ifdef Q_OS_WIN32
    file = QFileDialog::getOpenFileName(this, tr("Select an executable/script"), {}, u"Executable/script (*.exe *.bat)"_s);
#else
    file = QFileDialog::getOpenFileName(this, tr("Select a script file"), {}, u"Shell script (*.sh)"_s);
#endif
    if (!file.isEmpty())
        ui->pathLineEdit->setText(u'"' + file + u'"' + u" %file%");
}

#include "ShortcutCreatorDialog.h"
#include "ui_ShortcutCreatorDialog.h"

ShortcutCreatorDialog::ShortcutCreatorDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::ShortcutCreatorDialog)
{
    ui->setupUi(this);
    setWindowTitle(u"Add shortcut"_s);
    actionList = appActions->getList();
    scriptList = ScriptManager::scriptNames();

    ui->actionsComboBox->addItems(actionList);
    ui->actionsComboBox->setCurrentIndex(0);

    ui->scriptsComboBox->addItems(scriptList);
    ui->scriptsComboBox->setCurrentIndex(0);
}

ShortcutCreatorDialog::~ShortcutCreatorDialog()
{
    delete ui;
}

QString ShortcutCreatorDialog::selectedAction() const
{
    if (ui->actionsRadioButton->isChecked())
        return ui->actionsComboBox->currentText();
    else
        return u"s:"_sv + ui->scriptsComboBox->currentText();
}

QString ShortcutCreatorDialog::selectedShortcut() const
{
    return ui->sequenceEdit->sequence();
}

void ShortcutCreatorDialog::onShortcutEdited()
{
    QString action = actionManager->actionForShortcut(ui->sequenceEdit->sequence());
    if (!action.isEmpty())
        ui->warningLabel->setText(u"This shortcut is used for action: " + action + u". Replace?");
    else
        ui->warningLabel->setText(u""_s);
}

void ShortcutCreatorDialog::setAction(QString action)
{
    auto cbox = ui->actionsComboBox;
    if (action.startsWith(u"s:"_sv)) {
        action = action.remove(0, 2);
        cbox   = ui->scriptsComboBox;
        ui->scriptsRadioButton->setChecked(true);
    }
    int index = cbox->findText(action);
    if (index != -1)
        cbox->setCurrentIndex(index);
}

void ShortcutCreatorDialog::setShortcut(QString const &shortcut)
{
    ui->sequenceEdit->setText(shortcut);
}
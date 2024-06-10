#include "ShortcutCreatorDialog.h"
#include "ui_ShortcutCreatorDialog.h"

ShortcutCreatorDialog::ShortcutCreatorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ShortcutCreatorDialog)
{
    ui->setupUi(this);
    setWindowTitle(QS("Add shortcut"));
    actionList = appActions->getList();
    scriptList = scriptManager->scriptNames();

    ui->actionsComboBox->addItems(actionList);
    ui->actionsComboBox->setCurrentIndex(0);

    ui->scriptsComboBox->addItems(scriptList);
    ui->scriptsComboBox->setCurrentIndex(0);
}

ShortcutCreatorDialog::~ShortcutCreatorDialog() {
    delete ui;
}

QString ShortcutCreatorDialog::selectedAction() {
    if(ui->actionsRadioButton->isChecked())
        return ui->actionsComboBox->currentText();
    else
        return QS("s:")+ui->scriptsComboBox->currentText();
}

QString ShortcutCreatorDialog::selectedShortcut() {
    return ui->sequenceEdit->sequence();
}

void ShortcutCreatorDialog::onShortcutEdited() {
    QString action = actionManager->actionForShortcut(ui->sequenceEdit->sequence());
    if(!action.isEmpty())
        ui->warningLabel->setText(QS("This shortcut is used for action: ") + action + QS(". Replace?"));
    else
        ui->warningLabel->setText(QS(""));
}

void ShortcutCreatorDialog::setAction(QString action) {
    auto cbox = ui->actionsComboBox;
    if(action.startsWith(QS("s:"))) {
        action = action.remove(0,2);
        cbox = ui->scriptsComboBox;
        ui->scriptsRadioButton->setChecked(true);
    }
    int index = cbox->findText(action);
    if(index != -1)
       cbox->setCurrentIndex(index);
}

void ShortcutCreatorDialog::setShortcut(QString shortcut) {
    ui->sequenceEdit->setText(shortcut);
}

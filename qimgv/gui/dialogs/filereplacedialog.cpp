#include "filereplacedialog.h"
#include "ui_filereplacedialog.h"

FileReplaceDialog::FileReplaceDialog(QWidget *parent) : QDialog(parent), ui(new Ui::FileReplaceDialog) {
    ui->setupUi(this);
    multi = false;
    connect(ui->yesButton, &QPushButton::clicked, this, &FileReplaceDialog::onYesClicked);
    connect(ui->noButton, &QPushButton::clicked, this, &FileReplaceDialog::onNoClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &FileReplaceDialog::onCancelClicked);
}

FileReplaceDialog::~FileReplaceDialog() {
    delete ui;
}

void FileReplaceDialog::setSource(QString src) {
    ui->srcLabel->setText(src);
}

void FileReplaceDialog::setDestination(QString dst) {
    ui->dstLabel->setText(dst);
}

void FileReplaceDialog::setMode(FileReplaceMode mode) {
    if(mode == FILE_TO_FILE) {
        setWindowTitle(QS("File already exists"));
        ui->titleLabel->setText(QS("Replace destination file?"));
    } else if(mode == DIR_TO_DIR) {
        setWindowTitle(QS("Directory already exists"));
        ui->titleLabel->setText(QS("Merge directories?"));
    } else if(mode == DIR_TO_FILE) {
        setWindowTitle(QS("Destination already exists"));
        ui->titleLabel->setText(QS("There is a file with that name. Replace?"));
    } else { // FILE_TO_DIR
        setWindowTitle(QS("Destination already exists"));
        ui->titleLabel->setText(QS("There is a folder with that name. Replace?"));
    }
}

void FileReplaceDialog::setMulti(bool _multi) {
    multi = _multi;
    ui->applyAllCheckBox->setVisible(multi);
}

DialogResult FileReplaceDialog::getResult() const
{
    return result;
}

void FileReplaceDialog::onYesClicked() {
    result.yes = true;
    result.all = ui->applyAllCheckBox->isChecked();
    result.cancel = false;
    this->close();
}

void FileReplaceDialog::onNoClicked() {
    result.yes = false;
    result.all = ui->applyAllCheckBox->isChecked();
    result.cancel = false;
    this->close();
}

void FileReplaceDialog::onCancelClicked() {
    result.cancel = true;
    this->close();
}

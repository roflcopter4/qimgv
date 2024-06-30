#include "FileReplaceDialog.h"
#include "ui_FileReplaceDialog.h"

FileReplaceDialog::FileReplaceDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::FileReplaceDialog),
      multi(false)
{
    ui->setupUi(this);
    connect(ui->yesButton,    &QPushButton::clicked, this, &FileReplaceDialog::onYesClicked);
    connect(ui->noButton,     &QPushButton::clicked, this, &FileReplaceDialog::onNoClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &FileReplaceDialog::onCancelClicked);
}

FileReplaceDialog::~FileReplaceDialog()
{
    delete ui;
}

void FileReplaceDialog::setSource(QString const &src)
{
    ui->srcLabel->setText(src);
}

void FileReplaceDialog::setDestination(QString const &dst)
{
    ui->dstLabel->setText(dst);
}

void FileReplaceDialog::setMode(FileReplaceMode mode)
{
    switch (mode) {
    case FileReplaceMode::FILE_TO_FILE:
        setWindowTitle(u"File already exists"_s);
        ui->titleLabel->setText(u"Replace destination file?"_s);
        break;
    case FileReplaceMode::DIR_TO_DIR:
        setWindowTitle(u"Directory already exists"_s);
        ui->titleLabel->setText(u"Merge directories?"_s);
        break;
    case FileReplaceMode::DIR_TO_FILE:
        setWindowTitle(u"Destination already exists"_s);
        ui->titleLabel->setText(u"There is a file with that name. Replace?"_s);
        break;
    case FileReplaceMode::FILE_TO_DIR:
        setWindowTitle(u"Destination already exists"_s);
        ui->titleLabel->setText(u"There is a folder with that name. Replace?"_s);
        break;
    }
}

void FileReplaceDialog::setMulti(bool _multi)
{
    multi = _multi;
    ui->applyAllCheckBox->setVisible(multi);
}

DialogResult FileReplaceDialog::getResult() const
{
    return result;
}

void FileReplaceDialog::onYesClicked()
{
    result.yes    = true;
    result.all    = ui->applyAllCheckBox->isChecked();
    result.cancel = false;
    close();
}

void FileReplaceDialog::onNoClicked()
{
    result.yes    = false;
    result.all    = ui->applyAllCheckBox->isChecked();
    result.cancel = false;
    close();
}

void FileReplaceDialog::onCancelClicked()
{
    result.cancel = true;
    close();
}

#include "ResizeDialog.h"
#include "ui_ResizeDialog.h"

ResizeDialog::ResizeDialog(QSize initialSize, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::ResizeDialog),
      originalSize(initialSize),
      targetSize(initialSize),
      lastEdited(0)
{
    ui->setupUi(this);
    setWindowModality(Qt::ApplicationModal);
    ui->percent->setFocus();
    ui->width->setValue(initialSize.width());
    ui->height->setValue(initialSize.height());

    ui->resetButton->setText(tr("Reset:") + u' ' +
                             QString::number(initialSize.width()) +
                             u" x " +
                             QString::number(initialSize.height()));

    desktopSize = qApp->primaryScreen()->size();

    connect(ui->byPercentage,      &QRadioButton::toggled, this, &ResizeDialog::onPercentageRadioButton);
    connect(ui->byAbsoluteSize,    &QRadioButton::toggled, this, &ResizeDialog::onAbsoluteSizeRadioButton);
    connect(ui->keepAspectRatio,   &QCheckBox::toggled,    this, &ResizeDialog::onAspectRatioCheckbox);
    connect(ui->fitDesktopButton,  &QPushButton::pressed,  this, &ResizeDialog::fitDesktop);
    connect(ui->fillDesktopButton, &QPushButton::pressed,  this, &ResizeDialog::fillDesktop);
    connect(ui->resetButton,       &QPushButton::pressed,  this, &ResizeDialog::reset);
    connect(ui->cancelButton,      &QPushButton::pressed,  this, &ResizeDialog::reject);
    connect(ui->okButton,          &QPushButton::pressed,  this, &ResizeDialog::sizeSelect);

    connect(ui->percent,     qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ResizeDialog::percentChanged);
    connect(ui->width,       qOverload<int>(&QSpinBox::valueChanged),          this, &ResizeDialog::widthChanged);
    connect(ui->height,      qOverload<int>(&QSpinBox::valueChanged),          this, &ResizeDialog::heightChanged);
    connect(ui->resComboBox, qOverload<int>(&QComboBox::currentIndexChanged),  this, &ResizeDialog::setCommonResolution);
}

ResizeDialog::~ResizeDialog()
{
    delete ui;
}

void ResizeDialog::sizeSelect()
{
    if (targetSize != originalSize)
        emit sizeSelected(targetSize);
    accept();
}

void ResizeDialog::setCommonResolution(int index)
{
    QSize res;
    switch (index) {
    case 1:  res = QSize(1366, 768);  break;
    case 2:  res = QSize(1440, 900);  break;
    case 3:  res = QSize(1440, 1050); break;
    case 4:  res = QSize(1600, 1200); break;
    case 5:  res = QSize(1920, 1080); break;
    case 6:  res = QSize(1920, 1200); break;
    case 7:  res = QSize(2560, 1080); break;
    case 8:  res = QSize(2560, 1440); break;
    case 9:  res = QSize(2560, 1600); break;
    case 10: res = QSize(3840, 1600); break;
    case 11: res = QSize(3840, 2160); break;
    default: res = originalSize;      break;
    }
    targetSize = originalSize.scaled(res, ui->keepAspectRatio->isChecked() ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio);
    updateToTargetValues();
}

QSize ResizeDialog::newSize() const
{
    return targetSize;
}

void ResizeDialog::widthChanged(int newWidth)
{
    lastEdited    = 0;
    double factor = static_cast<double>(newWidth) / originalSize.width();
    targetSize.setWidth(newWidth);
    if (ui->keepAspectRatio->isChecked())
        targetSize.setHeight(static_cast<int>(originalSize.height() * factor));
    updateToTargetValues();
}

void ResizeDialog::heightChanged(int newHeight)
{
    lastEdited    = 1;
    double factor = static_cast<double>(newHeight) / originalSize.height();
    targetSize.setHeight(newHeight);
    if (ui->keepAspectRatio->isChecked())
        targetSize.setWidth(static_cast<int>(originalSize.width() * factor));
    updateToTargetValues();
}

void ResizeDialog::updateToTargetValues()
{
    ui->width->blockSignals(true);
    ui->height->blockSignals(true);
    ui->width->setValue(targetSize.width());
    ui->height->setValue(targetSize.height());
    ui->width->blockSignals(false);
    ui->height->blockSignals(false);
}

void ResizeDialog::fitDesktop()
{
    targetSize = originalSize.scaled(desktopSize, Qt::KeepAspectRatio);
    updateToTargetValues();
}

void ResizeDialog::fillDesktop()
{
    targetSize = originalSize.scaled(desktopSize, Qt::KeepAspectRatioByExpanding);
    updateToTargetValues();
}

void ResizeDialog::onAspectRatioCheckbox()
{
    resetResCheckBox();
    (lastEdited) ? heightChanged(ui->height->value()) : widthChanged(ui->width->value());
}

void ResizeDialog::onAbsoluteSizeRadioButton()
{
    ui->width->blockSignals(true);
    ui->height->blockSignals(true);
    ui->percent->blockSignals(true);
    ui->keepAspectRatio->blockSignals(true);

    ui->width->setEnabled(true);
    ui->height->setEnabled(true);
    ui->percent->setEnabled(false);
    ui->keepAspectRatio->setEnabled(true);

    ui->width->blockSignals(false);
    ui->height->blockSignals(false);
    ui->percent->blockSignals(false);
    ui->keepAspectRatio->blockSignals(false);
}

void ResizeDialog::onPercentageRadioButton()
{
    ui->width->blockSignals(true);
    ui->height->blockSignals(true);
    ui->percent->blockSignals(true);
    ui->keepAspectRatio->blockSignals(true);

    ui->width->setEnabled(false);
    ui->height->setEnabled(false);
    ui->percent->setEnabled(true);
    ui->keepAspectRatio->setChecked(true);
    ui->keepAspectRatio->setEnabled(false);
    percentChanged(ui->percent->value());

    ui->width->blockSignals(false);
    ui->height->blockSignals(false);
    ui->percent->blockSignals(false);
    ui->keepAspectRatio->blockSignals(false);
}

void ResizeDialog::resetResCheckBox()
{
    ui->resComboBox->blockSignals(true);
    ui->resComboBox->setCurrentIndex(0);
    ui->resComboBox->blockSignals(false);
}

void ResizeDialog::percentChanged(double newPercent)
{
    //double scale = newPercent / 100.;
    //targetSize.setWidth(originalSize.width() * scale);
    //targetSize.setHeight(originalSize.height() * scale);
    targetSize = originalSize * (newPercent / 100.0);
    updateToTargetValues();
}

void ResizeDialog::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        sizeSelect();
        break;
    case Qt::Key_Escape:
        reject();
        break;
    default:
        event->ignore();
        break;
    }
}

void ResizeDialog::reset()
{
    resetResCheckBox();
    targetSize = originalSize;
    updateToTargetValues();
}

int ResizeDialog::exec()
{
    resize(sizeHint());
    return QDialog::exec();
}

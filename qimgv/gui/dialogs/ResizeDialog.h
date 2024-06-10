#pragma once

#include <QCheckBox>
#include <QDebug>
#include <QScreen>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include "ui_ResizeDialog.h"

class ResizeDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit ResizeDialog(QSize initialSize, QWidget *parent = nullptr);
    ~ResizeDialog() override;

    ND QSize newSize() const;

  public slots:
    int exec() override;

  protected:
    void keyPressEvent(QKeyEvent *event) override;

  private:
    Ui::ResizeDialog *ui;
    QSize originalSize;
    QSize targetSize;
    QSize desktopSize;
    int   lastEdited; // 0 - width, 1 - height

    void  updateToTargetValues();
    void  resetResCheckBox();

  private slots:
    void widthChanged(int);
    void heightChanged(int);
    void percentChanged(double);
    void sizeSelect();

    void setCommonResolution(int);
    void reset();
    void fitDesktop();
    void fillDesktop();
    void onAspectRatioCheckbox();
    void onPercentageRadioButton();
    void onAbsoluteSizeRadioButton();

  signals:
    void sizeSelected(QSize);
};

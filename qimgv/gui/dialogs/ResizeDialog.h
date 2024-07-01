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
    DELETE_COPY_MOVE_ROUTINES(ResizeDialog);

    ND QSize newSize() const;

  protected:
    void keyPressEvent(QKeyEvent *event) override;

  private:
    void updateToTargetValues();
    void resetResCheckBox();

  Q_SIGNALS:
    void sizeSelected(QSize);

  public Q_SLOTS:
    int exec() override;

  private Q_SLOTS:
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

  private:
    Ui::ResizeDialog *ui;
    QSize originalSize;
    QSize targetSize;
    QSize desktopSize;
    int   lastEdited; // 0 - width, 1 - height
};

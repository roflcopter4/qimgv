#ifndef PRINTDIALOG_H
#define PRINTDIALOG_H

#include "Settings.h"
#include "components/thumbnailer/Thumbnailer.h"
#include <QDebug>
#include <QDialog>
#include <QFileDialog>
#include <QGraphicsColorizeEffect>
#include <QPainter>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrinterInfo>

namespace Ui {
class PrintDialog;
}

class PrintDialog : public QDialog {
    Q_OBJECT

public:
    explicit PrintDialog(QWidget *parent = nullptr);
    ~PrintDialog() override;
    void setImage(QSharedPointer<const QImage> const &_img);
    void setOutputPath(QString path);

private slots:
    void    print();
    void    exportPdf();
    QRectF  getImagePrintRect(QPrinter *printer);
    void    updatePreview();
    void    setLandscape(bool mode);
    void    onPrinterSelected(QString const &name);
    QString pdfPathDialog();

private:
    void saveSettings();
    Ui::PrintDialog *ui;
    QSharedPointer<const QImage> img = nullptr;
    QPrinter pdfPrinter, *printer = nullptr;
    bool printPdfDefault = false;
};

#endif // PRINTDIALOG_H

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

class PrintDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit PrintDialog(QWidget *parent);
    ~PrintDialog() override;

    void setImage(QSharedPointer<QImage const> const &newImg);
    void setOutputPath(QString path);

  private Q_SLOTS:
    void    print();
    void    exportPdf();
    QRectF  getImagePrintRect(QPrinter *printer);
    void    updatePreview();
    void    setLandscape(bool mode);
    void    onPrinterSelected(QString const &name);
    QString pdfPathDialog();

  private:
    void saveSettings();

    QSharedPointer<QImage const> img;
    Ui::PrintDialog *ui;
    QPrinter *printer;
    QPrinter  pdfPrinter;
    bool      printPdfDefault;
};

#endif // PRINTDIALOG_H

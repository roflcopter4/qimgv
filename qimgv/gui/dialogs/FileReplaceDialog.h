#pragma once
#include <QDialog>

struct DialogResult {
    bool yes    = false;
    bool all    = false;
    bool cancel = false;
    /*DIALOG_YES,
    DIALOG_YESTOALL,
    DIALOG_NO,
    DIALOG_NOTOALL,
    DIALOG_CANCEL*/
    bool operator==(bool const &cmp) const { return yes == cmp; }
    operator bool() const { return yes; }
};

enum class FileReplaceMode {
    FILE_TO_FILE,
    DIR_TO_DIR,
    FILE_TO_DIR,
    DIR_TO_FILE,
};

namespace Ui {
class FileReplaceDialog;
}

class FileReplaceDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit FileReplaceDialog(QWidget *parent = nullptr);
    ~FileReplaceDialog() override;

    void setMode(FileReplaceMode mode);
    void setMulti(bool);
    void setSource(QString const &src);
    void setDestination(QString const &dst);

    ND DialogResult getResult() const;

  private Q_SLOTS:
    void onYesClicked();
    void onNoClicked();
    void onCancelClicked();

  private:
    Ui::FileReplaceDialog *ui;
    DialogResult           result;
    bool                   multi;
};

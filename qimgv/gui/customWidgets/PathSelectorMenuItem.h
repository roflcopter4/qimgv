#pragma once

#include "gui/customWidgets/MenuItem.h"
#include "Settings.h"
#include <QFileDialog>

class PathSelectorMenuItem : public MenuItem
{
    Q_OBJECT

  public:
    explicit PathSelectorMenuItem(QWidget *parent);

    ND QString directory() const;
    ND QString path() const;

  protected:
    void onPress() override;

  Q_SIGNALS:
    void directorySelected(QString);

  public Q_SLOTS:
    void setDirectory(QString const &path);
    void onClicked();

  private:
    QString mDirectory;
    QString mPath;
};

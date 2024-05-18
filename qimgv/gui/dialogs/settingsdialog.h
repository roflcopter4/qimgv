#pragma once

#include <QDialog>
#include <QFileDialog>
#include <QColorDialog>
#include <QThreadPool>
#include <QTableWidget>
#include <QTextBrowser>
#include <QListWidget>
#include <QStackedWidget>
#include <QButtonGroup>
#include <QApplication>
#include <QDebug>
#include <QMenu>
#include "gui/customwidgets/colorselectorbutton.h"
#include "gui/dialogs/shortcutcreatordialog.h"
#include "gui/dialogs/scripteditordialog.h"
#include "settings.h"
#include "components/actionmanager/actionmanager.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog() override;
    void switchToPage(int number) const;

public slots:
    int exec() override;

private:
    void readColorScheme();
    void setColorScheme(ColorScheme colors) const;
    void saveColorScheme() const;
    void readSettings();
    void readShortcuts() const;
    void readScripts() const;


    void saveShortcuts() const;
    void addShortcutToTable(const QString &action, const QString &shortcut) const;
    void addScriptToList(const QString &name) const;
    void setupSidebar();
    void removeShortcutAt(int row) const;
    void adjustSizeToContents();

    Ui::SettingsDialog *ui;
    QMap<QString, QString> langs; // <"en_US", "English">
    QButtonGroup fitModeGrp;
    QButtonGroup folderEndGrp;
    QButtonGroup zoomIndGrp;

private slots:
    void saveSettings();
    void saveSettingsAndClose();

    void addScript();
    void editScript();
    void editScript(QListWidgetItem *item);
    void editScript(QString const &name);
    void removeScript();

    void addShortcut();
    void editShortcut();
    void editShortcut(int row) const;
    void removeShortcut() const;
    void resetShortcuts() const;
    void selectMpvPath();
    void onBgOpacitySliderChanged(int value) const;
    void onThumbnailerThreadsSliderChanged(int value) const;
    void onExpandLimitSliderChanged(int value) const;
    void onZoomStepSliderChanged(int value) const;
    void onJPEGQualitySliderChanged(int value) const;
    void resetToDesktopTheme();    
    void onAutoResizeLimitSliderChanged(int value) const;

    void resetZoomLevels() const;
signals:
    void settingsChanged();
};

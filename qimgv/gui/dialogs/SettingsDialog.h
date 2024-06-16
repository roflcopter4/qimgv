#pragma once

#include "Settings.h"
#include "components/actionManager/ActionManager.h"
#include "gui/customWidgets/ColorSelectorButton.h"
#include "gui/dialogs/ScriptEditorDialog.h"
#include "gui/dialogs/ShortcutCreatorDialog.h"
#include <QApplication>
#include <QButtonGroup>
#include <QColorDialog>
#include <QDebug>
#include <QDialog>
#include <QFileDialog>
#include <QListWidget>
#include <QMenu>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTextBrowser>
#include <QThreadPool>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit SettingsDialog(QWidget *parent);
    ~SettingsDialog() override;

    void switchToPage(int number) const;

  public Q_SLOTS:
    int exec() override;

  private:
    void readColorScheme();
    void setColorScheme(ColorScheme colors);
    void saveColorScheme() const;
    void readSettings();
    void readShortcuts() const;
    void readScripts() const;

    void saveShortcuts() const;
    void addShortcutToTable(QString const &action, QString const &shortcut) const;
    void addScriptToList(QString const &name) const;
    void setupSidebar();
    void removeShortcutAt(int row) const;
    void adjustSizeToContents();

    Ui::SettingsDialog    *ui;
    QMap<QString, QString> langs; // <"en_US", "English">
    QButtonGroup           fitModeGrp;
    QButtonGroup           folderEndGrp;
    QButtonGroup           zoomIndGrp;

  private Q_SLOTS:
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
    void onThemeSelectorComboBoxIndexChanged(int index);
    void onSystemColorsCheckBoxToggled(bool useSystemTheme);
    void onModifySystemSchemeLabelClicked();

  Q_SIGNALS:
    void settingsChanged();
};

#pragma once

#include "Settings.h"
#include "components/actionManager/ActionManager.h"
#include "gui/customWidgets/OverlayWidget.h"
#include "gui/customWidgets/PathSelectorMenuItem.h"
#include <QGridLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPushButton>

enum class CopyOverlayMode {
    COPY,
    MOVE,
};

namespace Ui
{
class CopyOverlay;
}

class CopyOverlay : public OverlayWidget
{
    Q_OBJECT

  public:
    explicit CopyOverlay(FloatingWidgetContainer *parent);
    ~CopyOverlay() override;

    void saveSettings();
    void setDialogMode(CopyOverlayMode _mode);

    ND CopyOverlayMode operationMode() const;

  Q_SIGNALS:
    void copyRequested(QString);
    void moveRequested(QString);

  public Q_SLOTS:
    void show();
    void hide();

  private Q_SLOTS:
    void requestFileOperation(QString const &path);
    void readSettings();

  protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool focusNextPrevChild(bool) override;

  private:
    void createDefaultPaths();
    void createPathWidgets();
    void createShortcuts();
    void removePathWidgets();

    static constexpr int maxPathCount = 9;

    Ui::CopyOverlay              *ui;
    QList<PathSelectorMenuItem *> pathWidgets;
    QStringList                   paths;
    QMap<QString, int>            shortcuts;
    CopyOverlayMode               mode;
};

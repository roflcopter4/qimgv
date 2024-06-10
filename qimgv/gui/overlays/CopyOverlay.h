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

namespace Ui {
    class CopyOverlay;
}

class CopyOverlay : public OverlayWidget {
    Q_OBJECT
public:
    CopyOverlay(FloatingWidgetContainer *parent);
    ~CopyOverlay() override;
    void saveSettings();    
    void setDialogMode(CopyOverlayMode _mode);
    CopyOverlayMode operationMode() const;

public slots:
    void show();
    void hide();

signals:
    void copyRequested(QString);
    void moveRequested(QString);

protected:
    void keyPressEvent(QKeyEvent *event) override;

    bool focusNextPrevChild(bool) override;
private slots:
    void requestFileOperation(QString const &path);
    void readSettings();

private:
    void createDefaultPaths();
    void createPathWidgets();
    void createShortcuts();
    Ui::CopyOverlay *ui;
    QList<PathSelectorMenuItem*> pathWidgets;
    const int maxPathCount = 9;
    QStringList paths;
    QMap<QString, int> shortcuts;
    CopyOverlayMode mode;
    void removePathWidgets();

};

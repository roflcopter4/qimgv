#pragma once

#include "Settings.h"
#include "gui/customWidgets/OverlayWidget.h"
#include <QLabel>
#include <QTimer>

#ifdef ERROR
# undef ERROR
#endif

namespace Ui {
class FloatingMessage;
}

enum class FloatingMessageIcon : uint8_t {
    NONE,
    DIRECTORY,
    LEFT_EDGE,
    RIGHT_EDGE,
    SUCCESS,
    WARNING,
    ERROR,
};

class FloatingMessage : public OverlayWidget
{
    Q_OBJECT

  public:
    explicit FloatingMessage(FloatingWidgetContainer *parent);
    ~FloatingMessage() override;

    void showMessage(QString const &text, FloatingMessageIcon icon, int duration);
    void showMessage(QString const &text, FloatingWidgetPosition Position, FloatingMessageIcon icon, int duration);

  protected:
    void mousePressEvent(QMouseEvent *event) override;

  public Q_SLOTS:
    void show();
    void setText(QString const &text);

  private Q_SLOTS:
    void readSettings();

  private:
    void doShowMessage(QString const &text, FloatingMessageIcon icon, int duration);
    void setIcon(FloatingMessageIcon icon);

    Ui::FloatingMessage   *ui;
    FloatingWidgetPosition preferredPosition;
    int                    hideDelay;
    QTimer                 visibilityTimer;
};

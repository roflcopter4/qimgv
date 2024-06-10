#pragma once

#include <QTimer>
#include <QLabel>
#include "gui/customwidgets/overlaywidget.h"
#include "Settings.h"

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

class FloatingMessage Q_DECL_FINAL : public OverlayWidget
{
    Q_OBJECT

  public:
    explicit FloatingMessage(FloatingWidgetContainer *parent);
    ~FloatingMessage() override;

    void showMessage(QString const &text, FloatingMessageIcon icon, int fadeDuration);
    void showMessage(QString const &text, FloatingWidgetPosition Position, FloatingMessageIcon icon, int duration);

  public slots:
    void show();
    void setText(QString text);

  private:
    QTimer                 visibilityTimer;
    Ui::FloatingMessage   *ui;
    int                    hideDelay;
    FloatingWidgetPosition preferredPosition;

    void doShowMessage(QString const &text, FloatingMessageIcon icon, int duration);
    void setIcon(FloatingMessageIcon icon);

  protected:
    void mousePressEvent(QMouseEvent *event) override;

  private slots:
    void readSettings();
};

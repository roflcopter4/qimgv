#pragma once

#include "Settings.h"
#include "gui/customWidgets/OverlayWidget.h"
#include <QLabel>
#include <QTimer>

#ifdef ERROR
# undef Error
#endif

namespace Ui {
class FloatingMessage;
}

class FloatingMessage : public OverlayWidget
{
    Q_OBJECT

  public:
    explicit FloatingMessage(FloatingWidgetContainer *parent);
    ~FloatingMessage() override;
    DELETE_COPY_MOVE_ROUTINES(FloatingMessage);

    enum class Icon : uint8_t {
        None,
        Directory,
        LeftEdge,
        RightEdge,
        Success,
        Warning,
        Error,
    };

    void showMessage(QString const &text, FloatingMessage::Icon icon, int duration);
    void showMessage(QString const &text, FloatingWidget::Position Position, FloatingMessage::Icon icon, int duration);

  protected:
    void mousePressEvent(QMouseEvent *event) override;

  private:
    void doShowMessage(QString const &text, FloatingMessage::Icon icon, int duration);
    void setIcon(FloatingMessage::Icon icon);

  public Q_SLOTS:
    void show();
    void setText(QString const &text);

  private Q_SLOTS:
    void readSettings();

  private:
    Ui::FloatingMessage *ui;
    QTimer visibilityTimer;
    int    hideDelay;
    FloatingWidget::Position preferredPosition;
};

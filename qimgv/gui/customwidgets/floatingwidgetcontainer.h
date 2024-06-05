#pragma once

#include <QVBoxLayout>
#include <QWidget>

class FloatingWidgetContainer : public QWidget
{
    Q_OBJECT

  public:
    explicit FloatingWidgetContainer(QWidget *parent = nullptr);
    ~FloatingWidgetContainer() override;

  Q_SIGNALS:
    void resized(QSize);

  protected:
    void resizeEvent(QResizeEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
};

// Base class for various menu items.
// Displays entry name, shortcut and an icon.

#pragma once

#include "components/actionManager/ActionManager.h"
#include "gui/customWidgets/IconButton.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QStyleOption>

class MenuItem : public QWidget
{
    Q_OBJECT

  public:
    explicit MenuItem(QWidget *parent = nullptr);
    ~MenuItem() override;

    void setText(QString const &text);
    void setShortcutText(QString const &text);
    void setIconPath(QString const &path);
    void setPassthroughClicks(bool mode);

    ND QString text() const;
    ND QString shortcut() const;

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    virtual void onClick();
    virtual void onPress();

    IconButton  *mIconWidget;
    QSpacerItem *spacer;
    QHBoxLayout *mLayout;

    QLabel mTextLabel;
    QLabel mShortcutLabel;
    bool   passthroughClicks = true;
};

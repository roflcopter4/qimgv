// Base class for various menu items.
// Displays entry name, shortcut and an icon.

#pragma once

#include "components/actionManager/ActionManager.h"
#include "gui/customwidgets/iconbutton.h"
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

    void    setText(QString const &text);
    QString text() const;
    void    setShortcutText(QString const &text);
    QString shortcut() const;
    void    setIconPath(QString const &path);
    void    setPassthroughClicks(bool mode);

  protected:
    IconButton  *mIconWidget;
    QSpacerItem *spacer;
    QHBoxLayout *mLayout;

    QLabel mTextLabel;
    QLabel mShortcutLabel;
    bool   passthroughClicks = true;

    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    virtual void onClick();
    virtual void onPress();
};

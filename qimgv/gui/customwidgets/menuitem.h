// Base class for various menu items.
// Displays entry name, shortcut and an icon.

#pragma once

#include "components/actionmanager/actionmanager.h"
#include "gui/customwidgets/iconbutton.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QStyleOption>

class MenuItem : public QWidget
{
    Q_OBJECT

  public:
    MenuItem(QWidget *parent = nullptr);
    ~MenuItem() override;

    void    setText(QString const &mTextLabel);
    QString text() const;
    void    setShortcutText(QString const &mTextLabel);
    QString shortcut() const;
    void    setIconPath(QString const &path);
    void    setPassthroughClicks(bool mode);

  protected:
    IconButton   mIconWidget;
    QLabel       mTextLabel;
    QLabel       mShortcutLabel;
    QSpacerItem *spacer;
    QHBoxLayout  mLayout;
    bool         passthroughClicks = true;

    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    virtual void onClick();
    virtual void onPress();
};

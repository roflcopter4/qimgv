#pragma once

#include "gui/customWidgets/IconWidget.h"
#include <QBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QStyleOption>
#include <QWidget>

class SSideBarItem;

class SSideBar : public QWidget
{
    Q_OBJECT

  public:
    explicit SSideBar(QWidget *parent = nullptr);
    void addEntry(QString const &icon, QString const &name);
    void selectEntry(qsizetype idx);

  private:
    QBoxLayout           *layout;
    QList<SSideBarItem *> entries;
    Qt::Orientation       orientation = Qt::Vertical;

    void selectEntryAt(QPoint pos);

  Q_SIGNALS:
    void entrySelected(qsizetype);

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void paintEvent(QPaintEvent *event) override;
};

class SSideBarItem : public QWidget
{
    Q_OBJECT

  public:
    explicit SSideBarItem(QString const &icon, QString const &name, QWidget *parent = nullptr);
    void     setHighlighted(bool mode);
    ND bool  highlighted() const;

  private:
    QBoxLayout *layout;
    IconWidget  iconWidget;
    QLabel      textLabel;
    bool        mHighlighted = false;

  protected:
    void paintEvent(QPaintEvent *event) override;
};

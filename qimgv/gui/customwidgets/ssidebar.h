#ifndef SSIDEBAR_H
#define SSIDEBAR_H

#include <QWidget>
#include <QLabel>
#include <QPainter>
#include <QStyleOption>
#include <QBoxLayout>
#include "gui/customwidgets/iconwidget.h"

class SSideBarItem;

class SSideBar : public QWidget {
    Q_OBJECT
public:
    explicit SSideBar(QWidget *parent = nullptr);
    void     addEntry(QString const &icon, QString const &name);
    void     selectEntry(int idx);

private:
    QBoxLayout *layout;
    QList<SSideBarItem *> entries;
    Qt::Orientation orientation = Qt::Vertical;
    void selectEntryAt(QPoint pos);

signals:
    void entrySelected(int);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void paintEvent(QPaintEvent *event) override;
};

class SSideBarItem : public QWidget {
    Q_OBJECT
public:
    explicit SSideBarItem(QString icon, QString name, QWidget *parent = nullptr);
    void setHighlighted(bool mode);
    bool highlighted();

private:
    QBoxLayout *layout;
    IconWidget iconWidget;
    QLabel textLabel;
    bool mHighlighted = false;

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // SSIDEBAR_H

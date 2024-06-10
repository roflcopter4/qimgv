#pragma once

#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include <QWheelEvent>
#include <QWidget>

namespace Ui {
class InfoBar;
}

class InfoBar : public QWidget
{
    Q_OBJECT

public:
    explicit InfoBar(QWidget *parent = nullptr);
    ~InfoBar() override;

public slots:
    void setInfo(QString const &position, QString const &fileName, QString const &info);
protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
private:
    Ui::InfoBar *ui;
};

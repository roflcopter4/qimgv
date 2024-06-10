#pragma once

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QStyleOption>
#include <QPainter>
#include <QDebug>

class EntryInfoItem final : public QWidget
{
    Q_OBJECT

  public:
    explicit EntryInfoItem(QWidget *parent = nullptr);
    void setInfo(QString newName, QString newValue);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    QHBoxLayout *layout;
    QString     name;
    QString     value;
    QLabel      nameLabel;
    QLabel      valueLabel;
};

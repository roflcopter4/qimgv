#pragma once

#include <QApplication>
#include <QProxyStyle>

class ProxyStyle : public QProxyStyle
{
    Q_OBJECT;

  public:
    explicit ProxyStyle(QStyle *parent) : QProxyStyle(parent) {}
    ~ProxyStyle() override = default;

    void drawPrimitive(PrimitiveElement    element,
                       QStyleOption const *option,
                       QPainter           *painter,
                       QWidget const      *widget = nullptr) const override;
};

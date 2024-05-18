#pragma once

#include <QApplication>
#include <QProxyStyle>

class ProxyStyle : public QProxyStyle {
public:
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;
};

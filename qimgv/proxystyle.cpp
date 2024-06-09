#include "ProxyStyle.h"

void ProxyStyle::drawPrimitive(
      QStyle::PrimitiveElement element,
      QStyleOption const      *option,
      QPainter                *painter,
      QWidget const           *widget) const
{
    if (PE_FrameFocusRect == element) {
        // do not draw focus rectangle
    } else {
        QProxyStyle::drawPrimitive(element, option, painter, widget);
    }
}

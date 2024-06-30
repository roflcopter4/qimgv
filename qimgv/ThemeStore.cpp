#include "ThemeStore.h"

ColorScheme ThemeStore::colorScheme(ColorSchemes name)
{
    BaseColorScheme base = {-1};
    QPalette        p;

    switch (name) {
    case ColorSchemes::SYSTEM:
    case ColorSchemes::CUSTOMIZED:
        base.folderview_topbar = p.window().color();
        base.widget            = p.window().color();
        base.widget_border     = p.window().color();
        base.folderview        = p.base().color();
        base.text              = p.text().color();
        base.icons             = p.text().color();
        base.accent            = p.highlight().color();
        base.scrollbar.setHsv(p.highlight().color().hue(),
                              qBound(0, p.highlight().color().saturation() - 20, 240),
                              qBound(0, p.highlight().color().value() - 35, 240));
        base.tid = static_cast<int>(name);
        break;
    case ColorSchemes::LIGHT: // v2, works with w10 titlebars
        base.accent                = QColor(u"#719ccd"_sv);
        base.background            = QColor(u"#1a1a1a"_sv);
        base.background_fullscreen = QColor(u"#1a1a1a"_sv);
        base.folderview            = QColor(u"#f2f2f2"_sv);
        base.folderview_topbar     = QColor(u"#ffffff"_sv);
        base.icons                 = QColor(u"#656768"_sv);
        base.overlay               = QColor(u"#1a1a1a"_sv);
        base.overlay_text          = QColor(u"#d2d2d2"_sv);
        base.text                  = QColor(u"#353535"_sv);
        base.scrollbar             = QColor(u"#aaaaaa"_sv);
        base.widget                = QColor(u"#ffffff"_sv);
        base.widget_border         = QColor(u"#c3c3c3"_sv);
        base.tid                   = static_cast<int>(name);
        break;
    case ColorSchemes::DARKBLUE:
        base.background            = QColor(u"#18191a"_sv);
        base.background_fullscreen = QColor(u"#18191a"_sv);
        base.text                  = QColor(u"#cdd2d7"_sv);
        base.icons                 = QColor(u"#babec3"_sv);
        base.widget                = QColor(u"#232629"_sv);
        base.widget_border         = QColor(u"#26292d"_sv);
        base.accent                = QColor(u"#336ca5"_sv);
        base.folderview            = QColor(u"#232629"_sv);
        base.folderview_topbar     = QColor(u"#31363b"_sv);
        base.scrollbar             = QColor(u"#4f565c"_sv);
        base.overlay_text          = QColor(u"#d2d2d2"_sv);
        base.overlay               = QColor(u"#1a1a1a"_sv);
        base.tid                   = static_cast<int>(name);
        break;
    case ColorSchemes::BLACK:
        base.background            = QColor(u"#000000"_sv);
        base.background_fullscreen = QColor(u"#000000"_sv);
        base.text                  = QColor(u"#b0b0b0"_sv);
        base.icons                 = QColor(u"#999999"_sv);
        base.widget                = QColor(u"#080808"_sv);
        base.widget_border         = QColor(u"#181818"_sv);
        base.accent                = QColor(u"#5a5a5a"_sv);
        base.folderview            = QColor(u"#111111"_sv);
        base.folderview_topbar     = QColor(u"#111111"_sv);
        base.scrollbar             = QColor(u"#343434"_sv);
        base.overlay_text          = QColor(u"#999999"_sv);
        base.overlay               = QColor(u"#000000"_sv);
        base.tid                   = static_cast<int>(name);
        break;
    case ColorSchemes::DARK:
        base.background            = QColor(u"#1a1a1a"_sv);
        base.background_fullscreen = QColor(u"#1a1a1a"_sv);
        base.text                  = QColor(u"#b6b6b6"_sv);
        base.icons                 = QColor(u"#a4a4a4"_sv);
        base.widget                = QColor(u"#252525"_sv);
        base.widget_border         = QColor(u"#2c2c2c"_sv);
        base.accent                = QColor(u"#8c9b81"_sv);
        base.folderview            = QColor(u"#242424"_sv);
        base.folderview_topbar     = QColor(u"#383838"_sv);
        base.scrollbar             = QColor(u"#5a5a5a"_sv);
        base.overlay_text          = QColor(u"#d2d2d2"_sv);
        base.overlay               = QColor(u"#1a1a1a"_sv);
        base.tid                   = static_cast<int>(name);
        break;
    }

    return {base};
}

//---------------------------------------------------------------------

ColorScheme::ColorScheme()
{
    tid = -1;
}

ColorScheme::ColorScheme(BaseColorScheme const &base)
{
    setBaseColors(base);
}

void ColorScheme::setBaseColors(BaseColorScheme const &base)
{
    background            = base.background;
    background_fullscreen = base.background_fullscreen;
    text                  = base.text;
    icons                 = base.icons;
    widget                = base.widget;
    widget_border         = base.widget_border;
    accent                = base.accent;
    folderview            = base.folderview;
    folderview_topbar     = base.folderview_topbar;
    overlay               = base.overlay;
    overlay_text          = base.overlay_text;
    scrollbar             = base.scrollbar;
    tid                   = base.tid;
    createColorVariants();
}

void ColorScheme::createColorVariants()
{
    if (widget.valueF() <= 0.45f) { // dark theme
        // top bar buttons
        panel_button.setHsv(folderview_topbar.hue(), folderview_topbar.saturation(), qMin(folderview_topbar.value() + 20, 255));
        panel_button_hover.setHsv(folderview_topbar.hue(), folderview_topbar.saturation(), qMin(folderview_topbar.value() + 26, 255));
        panel_button_pressed.setHsv(folderview_topbar.hue(), folderview_topbar.saturation(), qMin(folderview_topbar.value() + 15, 255));
        folderview_hc.setHsv(folderview.hue(), folderview.saturation(), qMin(folderview.value() + 12, 255));
        folderview_hc2.setHsv(folderview.hue(), folderview.saturation(), qMin(folderview.value() + 28, 255));
        folderview_button_pressed = folderview_hc;
        folderview_button_hover   = folderview_hc2;

        // regular buttons - from widget bg
        button.setHsv(widget.hue(), widget.saturation(), qMin(widget.value() + 21, 255));
        button_hover    = QColor(button.lighter(112));
        button_pressed  = QColor(button.darker(112));
        scrollbar_hover = scrollbar.lighter(120);

        // text
        text_hc  = QColor(text.lighter(110));
        text_hc2 = QColor(text.lighter(118));
        text_lc  = QColor(text.darker(115));
        text_lc2 = QColor(text.darker(160));
    } else { // light theme
        // top bar buttons
        panel_button.setHsv(folderview_topbar.hue(), folderview_topbar.saturation(), qMax(folderview_topbar.value() - 30, 0));
        panel_button_hover.setHsv(folderview_topbar.hue(), folderview_topbar.saturation(), qMax(folderview_topbar.value() - 45, 0));
        panel_button_pressed.setHsv(folderview_topbar.hue(), folderview_topbar.saturation(), qMax(folderview_topbar.value() - 55, 0));
        folderview_hc.setHsv(folderview.hue(), folderview.saturation(), qMax(folderview.value() - 25, 0));
        folderview_hc2.setHsv(folderview.hue(), folderview.saturation(), qMax(folderview.value() - 60, 0));
        folderview_button_pressed = folderview_hc2;
        folderview_button_hover   = folderview_hc;
        // regular buttons - from widget bg
        button.setHsv(widget.hue(), widget.saturation(), qMax(widget.value() - 42, 0));
        button_hover    = QColor(button.darker(106));
        button_pressed  = QColor(button.darker(118));
        scrollbar_hover = scrollbar.darker(120);
        // text
        text_hc  = QColor(text.darker(104));
        text_hc2 = QColor(text.darker(112));
        text_lc  = QColor(text.lighter(130));
        text_lc2 = QColor(text.lighter(160));
    }

    // misc
    input_field_focus = QColor(accent);
}

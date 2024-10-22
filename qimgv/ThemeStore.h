#pragma once

#include <QColor>
#include <QDebug>
#include <QPalette>

enum class ColorSchemes {
    SYSTEM,
    LIGHT,
    BLACK,
    DARK,
    DARKBLUE,
    CUSTOMIZED,
};

struct BaseColorScheme {
    int    tid;
    QColor background;
    QColor background_fullscreen;
    QColor text;
    QColor icons;
    QColor widget;
    QColor widget_border;
    QColor accent;
    QColor folderview;
    QColor folderview_topbar;
    QColor scrollbar;
    QColor overlay_text;
    QColor overlay;
};

class ColorScheme
{
  public:
    ColorScheme();
    explicit ColorScheme(BaseColorScheme const &base);
    void setBaseColors(BaseColorScheme const &base);

    // index of theme name
    int tid;
    // base
    QColor background;
    QColor background_fullscreen;
    QColor text;
    QColor icons;
    QColor widget;
    QColor widget_border;
    QColor accent;
    QColor folderview;
    QColor folderview_topbar;
    QColor scrollbar;
    QColor scrollbar_hover;
    QColor overlay_text;
    QColor overlay;
    // extended
    QColor text_hc2;
    QColor text_hc;
    QColor text_lc;
    QColor text_lc2;
    QColor button;
    QColor button_hover;
    QColor button_pressed;
    QColor panel_button;
    QColor panel_button_hover;
    QColor panel_button_pressed;
    QColor folderview_hc;
    QColor folderview_hc2;
    QColor folderview_button_hover;
    QColor folderview_button_pressed;
    QColor input_field_focus;

  private:
    void createColorVariants();
};

class ThemeStore
{
  public:
    static ColorScheme colorScheme(ColorSchemes name);
};

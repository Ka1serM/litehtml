#ifndef LITEHTML_CSS_CONTROL_H
#define LITEHTML_CSS_CONTROL_H

#include "web_color.h"
#include "css_values.h"

namespace litehtml
{
    struct css_scrollbar_colors
    {
        web_color thumb = web_color::transparent;
        web_color track = web_color::transparent;
        bool       auto_value = true;
    };

    inline constexpr auto scrollbar_width_strings = split_css_values<3>("auto;thin;none");

    enum scrollbar_width
    {
        scrollbar_width_auto,
        scrollbar_width_thin,
        scrollbar_width_none,
    };

    struct css_accent_color
    {
        web_color color = web_color::transparent;
        bool       auto_value = true;
    };
}

#endif

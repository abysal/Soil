//
// Created by Akashi on 02/03/25.
//

#ifndef EDITOR_THEME_HPP
#define EDITOR_THEME_HPP
#include "../types.hpp"
#include "./header_bar.hpp"

namespace soil {
struct EditorVisualConfig {
    Color header_color_base = Color("1A1C1C");
    Color header_button_color = Color("2B272B");
    Color header_stripe_color = Color("916C80");
    Color base_color = Color("292928");
    Color normal_text_color = Color("FEFDFB");

    BarHeight bar_height = BarHeight::Normal;
};
} // namespace soil

#endif // EDITOR_THEME_HPP

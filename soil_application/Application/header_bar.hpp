//
// Created by Akashi on 02/03/25.
//

#ifndef HEADER_BAR_HPP
#define HEADER_BAR_HPP
#include "../types.hpp"

namespace soil {
    enum class BarHeight : u16 {
        Small      = 20,
        Normal     = 30,
        Large      = 40,
        ExtraLarge = 80,
    };

    enum class ButtonGapSize { Small = 8, Normal = 16, Large = 24 };

    enum class GeneralGapSize { Small = 8, Normal = 16, Large = 24 };
} // namespace soil

#endif // HEADER_BAR_HPP

#include "texture.hpp"

namespace soil {
    ApiTexture gradient(uint32_t width, uint32_t height, Rml::Colourb start, Rml::Colourb end) {
        std::byte* data = new std::byte[width * height * 4];

        double progress = 0.0;

        const auto prim_height  = height;
        const auto other_height = width;

        const double step_size = [] {
            auto out_val = 1.0 / static_cast<double>(prim_height);

            if (out_val <= 0) {
                out_val = std::numeric_limits<double>::epsilon();
            }

            return out_val;
        }();

        static_assert(sizeof(start) == 4);

        for (uint32_t y = 0; y < prim_height; y++) {
            const auto color = lerp(start, end, progress);
            auto*      row   = get_row<Rml::Colourb>(data, y, width);

            const auto color_data = std::bit_cast<std::array<std::byte, 4>>(color);

            for (uint32_t x = 0; x < other_height; x++) {
                memcpy(row, color_data.data(), sizeof(Rml::Colourb));
                row += sizeof(Rml::Colourb);
            }
            progress += step_size;
        }

        return ApiTexture{
            .width = width, .height = height, .data = data, .format = TextureFormat::RGBA8
        };
    }
} // namespace soil
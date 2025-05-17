#include "texture.hpp"

#include <glm/glm.hpp>

namespace soil {
    ApiTexture gradient(uint32_t width, uint32_t height, glm::u8vec4 start, glm::u8vec4 end) {
        std::byte* data = new std::byte[width * height * 4];

        double progress = 0.0;

        const auto prim_height  = height;
        const auto other_height = width;

        const double step_size = [=] {
            auto out_val = 1.0 / static_cast<double>(prim_height);

            if (out_val <= 0) {
                out_val = std::numeric_limits<double>::epsilon();
            }

            return out_val;
        }();

        static_assert(sizeof(start) == 4);

        for (uint32_t y = 0; y < prim_height; y++) {
            glm::vec4 interpolated = glm::mix(glm::vec4(start), glm::vec4(end), progress);
            auto      color        = glm::u8vec4(interpolated);
            auto*     row          = get_row<Rml::Colourb>(data, y, width);

            const auto color_data = std::bit_cast<std::array<std::byte, 4>>(color);

            for (uint32_t x = 0; x < other_height; x++) {
                memcpy(row, color_data.data(), sizeof(Rml::Colourb));
                row += sizeof(Rml::Colourb);
            }
            progress = std::clamp(progress + step_size, 0.0, 1.0);
        }

        return ApiTexture{width, height, data, TextureFormat::RGBA8};
    }
} // namespace soil
#include "render.hpp"
#include "../types.hpp"
#include "./clay_binding.hpp"
#include "./components.hpp"
#include <stdexcept>

// TODO:
//      Implement a better text rendering using a bump allocator
//      Render raw OpenGl texture

namespace clay_extension {
    void render_command_list(
        Clay_RenderCommandArray commands, std::span<Font> fonts
    ) noexcept {
        for (usize index = 0; index < commands.length; index++) {
            Clay_RenderCommand &command = commands.internalArray[index];

            if (command.commandType !=
                Clay_RenderCommandType::CLAY_RENDER_COMMAND_TYPE_CUSTOM) {
                raylib_render_command_passthrough(
                    Clay_RenderCommandArray{1, 1, &command}, fonts.data()
                );
                continue;
            }

            const auto *type = (ComponentType *)command.userData;

            if (*type == ComponentType::RAYLIB_3D_MODEL) {
                raylib_render_command_passthrough(
                    Clay_RenderCommandArray{1, 1, &command}, fonts.data()
                );
                continue;
            }

            if (*type == ComponentType::CUSTOM_VIRTUAL) {
                const auto *data = (RenderComponentStore *)command.userData;

                data->component->on_render();
                continue;
            }

            // This throw is fine since we want to kill the application anyway.
            // So its ok to just call std::terminate
            // NOLINTNEXTLINE
            throw std::logic_error("Unhandled Draw Command");
        }
    }
} // namespace clay_extension
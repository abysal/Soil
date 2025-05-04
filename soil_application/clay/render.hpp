#pragma once
#include "clay.h"
#include <raylib.h>
#include <span>
#include <string_view>

namespace raylib_renderer {

    enum CustomLayoutElementType { CUSTOM_LAYOUT_ELEMENT_TYPE_3D_MODEL };
    void CustomDrawTextEx(
        Font font, std::string_view text, Vector2 position, float fontSize, float spacing,
        Color tint
    );

    struct CustomLayoutElement_3DModel {
        Model   model;
        float   scale;
        Vector3 position;
        Matrix  rotation;
    };

    struct CustomLayoutElement {
        CustomLayoutElementType type;
        union {
            CustomLayoutElement_3DModel model;
        } customData;
    };

    void Clay_Raylib_Render(Clay_RenderCommandArray renderCommands, Font* fonts) noexcept;
    void Clay_Raylib_Initialize(
        int width, int height, const char* title, unsigned int flags
    ) noexcept;
    Clay_Dimensions Raylib_MeasureText(
        Clay_StringSlice text, Clay_TextElementConfig* config, void* userData
    ) noexcept;
} // namespace raylib_renderer

namespace clay_extension {
    void render_command_list(Clay_RenderCommandArray commands, std::span<Font> fonts) noexcept;

}
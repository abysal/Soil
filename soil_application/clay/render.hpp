#pragma once
#include "clay.h"
#include <raylib.h>
#include <span>

namespace raylib_renderer {

    typedef enum { CUSTOM_LAYOUT_ELEMENT_TYPE_3D_MODEL } CustomLayoutElementType;

    typedef struct {
        Model   model;
        float   scale;
        Vector3 position;
        Matrix  rotation;
    } CustomLayoutElement_3DModel;

    typedef struct {
        CustomLayoutElementType type;
        union {
            CustomLayoutElement_3DModel model;
        } customData;
    } CustomLayoutElement;

    void Clay_Raylib_Render(Clay_RenderCommandArray renderCommands, Font *fonts) noexcept;
    void Clay_Raylib_Initialize(
        int width, int height, const char *title, unsigned int flags
    ) noexcept;
    Clay_Dimensions Raylib_MeasureText(
        Clay_StringSlice text, Clay_TextElementConfig *config, void *userData
    ) noexcept;
} // namespace raylib_renderer

namespace clay_extension {
    void render_command_list(Clay_RenderCommandArray commands, std::span<Font> fonts) noexcept;

}
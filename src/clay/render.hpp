#pragma once
#include "clay.h"
#include <raylib.h>
#include <span>

namespace clay_extension {
    void render_command_list(Clay_RenderCommandArray commands, std::span<Font> fonts) noexcept;   
}
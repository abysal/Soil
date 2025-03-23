//
// Created by Akashi on 02/03/25.
//
#include "types.hpp"
#include <optional>
#define CLAY_IMPLEMENTATION
#define RAY_IMPLEMENTATION
#include "./raywrapped.hpp"
#include "clay_binding.hpp"

#include <memory>

#include "../Application/jet_brains_mono.hpp"
#include "../Application/ttf/simple_ttf.hpp"
#include "./render.hpp"
#include "./simple_id_generator.hpp"
#include "./templates.hpp"
#include <print>

static void log_handle_error(Clay_ErrorData data) { std::println("Error: {}", data.errorText); }

// TODO:
//      Implement a font manager which handles rendering text a million times better
std::unique_ptr<Font> setup_basics(const char *window_title) noexcept {
    Clay_SetMaxElementCount(65535 * 32);
    raylib_renderer::Clay_Raylib_Initialize(
        512, 512, window_title, FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_WINDOW_MAXIMIZED
    );

    const i32 monitor = GetCurrentMonitor();

    const i32 width  = static_cast<i32>(static_cast<float>(GetMonitorWidth(monitor)) * 0.9);
    const i32 height = static_cast<i32>(static_cast<float>(GetMonitorHeight(monitor)) * 0.9);

    SetWindowSize(width, height);

    const uint32_t memory_size = Clay_MinMemorySize();

    Clay_Initialize(
        Clay_CreateArenaWithCapacityAndMemory(memory_size, malloc(memory_size)),
        {
            .width  = static_cast<f32>(width),
            .height = static_cast<f32>(height),
        },
        {.errorHandlerFunction = &log_handle_error, .userData = nullptr}
    );

    // The U8 cast is legal since we know the from_data call doesn't modify the
    // underlying data (it takes a const version) So there is no UB by writing
    // to the constant memory. This function call will also never fail since we
    // know this TTF is valid. Also the fact this TTF file is entirely constexpr parsed is nuts
    constexpr soil::TTF file =
        soil::TTF::from_data({const_cast<u8 *>(raw_hex.data()), raw_hex.size()});

    const auto font = LoadFontFromMemory(
        ".ttf", raw_hex.data(), raw_hex.size(), 15, nullptr, file.glyphs_count()
    );
    auto render_font = std::make_unique<Font>(font);

    Clay_SetMeasureTextFunction(&raylib_renderer::Raylib_MeasureText, render_font.get());

    return render_font;
}

static void per_frame_clay_update() noexcept {
    const u32 width  = GetScreenWidth();
    const u32 height = GetScreenHeight();

    const auto mouse = GetMousePosition();

    auto mouse_scroll_delta  = GetMouseWheelMoveV();
    mouse_scroll_delta.y    *= 100;

    Clay_SetLayoutDimensions({static_cast<f32>(width), static_cast<f32>(height)});

    Clay_SetPointerState(std::bit_cast<Clay_Vector2>(mouse), IsMouseButtonPressed(0));

    Clay_UpdateScrollContainers(
        true, std::bit_cast<Clay_Vector2>(mouse_scroll_delta), GetFrameTime()
    );

    soil::ClayIdGenerator::instance().wipe();
}

void render_loop(std::function<void()> &&func, std::unique_ptr<Font> font) noexcept {
    while (!WindowShouldClose()) {
        per_frame_clay_update();

        Clay_BeginLayout();
        func();
        const auto commands = Clay_EndLayout();

        BeginDrawing();
        ClearBackground(BLACK);
        clay_extension::render_command_list(commands, std::span<Font>{font.get(), 1});
        EndDrawing();
    }
}

namespace clay_extension {

    Clay_ElementId owner_id() noexcept {
        const auto hash = Clay__GetParentElementId();
        return {.id = hash, .stringId = {.length = 0, .chars = nullptr}};
    }

    void raylib_render_command_passthrough(
        Clay_RenderCommandArray renderCommands, Font *fonts
    ) noexcept {
        raylib_renderer::Clay_Raylib_Render(renderCommands, fonts);
    }

    bool ButtonConfig::render_button(
        std::string_view text, Clay_ElementId id,
        std::optional<std::function<void(const ButtonConfig &, std::string_view)>> &&callback,
        bool                                                                         bare
    ) const noexcept {
        const auto hovered = this->is_hovered(id);
        const auto config  = this->create_decleration(hovered, id);

        const auto func = [&] {
            CLAY_TEXT(
                to_clay(text),
                CLAY_TEXT_CONFIG({.textColor = this->text_color, .fontSize = this->font_size})
            );

            if (callback.has_value()) callback.value()(*this, text);
        };

        if (!bare) {
            new_element(config, func);
        } else {
            func();
        }

        return hovered && IsMouseButtonPressed(0);
    }

    Clay_ElementDeclaration
    ButtonConfig::create_decleration(bool hovered, const Clay_ElementId &id) const noexcept {

        const soil::Color color =
            (hovered) ? this->background_color * (this->hover_light_amount + 1.0f)
                      : this->background_color;

        const Clay_SizingMinMax min_max_x = {
            .min = static_cast<f32>(this->min_size.x), .max = 21932810932.f
        };

        const Clay_SizingMinMax min_max_y = {
            .min = static_cast<f32>(this->min_size.y), .max = 21932810932.f
        };

        const Clay_ElementDeclaration config = {
            .id = id,
            .layout{
                .sizing{
                    .width  = CLAY_SIZING_FIT(min_max_x),
                    .height = CLAY_SIZING_FIT(min_max_y),

                },
                .padding = CLAY_PADDING_ALL(this->text_padding)
            },
            .backgroundColor = color,
            .cornerRadius    = {this->rounding, this->rounding, this->rounding, this->rounding},
        };

        return config;
    }

    void
    new_element(Clay_ElementDeclaration declaration, std::function<void()> &&inner) noexcept {

        if (declaration.id.id == 0) {
            declaration.id = soil::ClayIdGenerator::instance().new_id();
        }

        Clay__OpenElement();
        Clay__ConfigureOpenElement(declaration);
        inner();
        Clay__CloseElement();
    }
} // namespace clay_extension

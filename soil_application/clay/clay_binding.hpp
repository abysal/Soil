#pragma once
#include "raywrapped.hpp"

#include "../types.hpp"
#include "Application/memory/fallback_allocator.hpp"
#include "Application/memory/pointer.hpp"
#include <clay.h>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <memory>
#include <optional>
#include <string_view>

std::unique_ptr<Font> setup_basics(const char* window_title) noexcept;

void render_loop(std::function<void()>&& func, std::unique_ptr<Font> font) noexcept;

namespace clay_extension {

    inline soil::BlockAllocator<8192, 1> CLAY_COPY_BUFFER{};

    inline std::observer_ptr<Clay_TextElementConfig> text_config(Clay_TextElementConfig cfg
    ) noexcept {
        return Clay__StoreTextElementConfig(cfg);
    }

    void raylib_render_command_passthrough(
        Clay_RenderCommandArray renderCommands, Font* fonts
    ) noexcept;

    Clay_ElementId owner_id() noexcept;

    struct ClayElementDeclarationPartial {
        // Controls various settings that affect the size and position of an
        // element, as well as the sizes and positions of any child elements.
        Clay_LayoutConfig layout;
        // Controls the background color of the resulting element.
        // By convention specified as 0-255, but interpretation is up to the
        // renderer. If no other config is specified, .backgroundColor will
        // generate a RECTANGLE render command, otherwise it will be passed as a
        // property to IMAGE or CUSTOM render commands.
        Clay_Color backgroundColor;
        // Controls the "radius", or corner rounding of elements, including
        // rectangles, borders and images.
        Clay_CornerRadius cornerRadius;
        // Controls settings related to image elements.
        Clay_ImageElementConfig image;
        // Controls whether and how an element "floats", which means it layers
        // over the top of other elements in z order, and doesn't affect the
        // position and size of siblings or parent elements. Note: in order to
        // activate floating, .floating.attachTo must be set to something other
        // than the default value.
        Clay_FloatingElementConfig floating;
        // Used to create CUSTOM render commands, usually to render element
        // types not supported by Clay.
        Clay_CustomElementConfig custom;
        // Controls whether an element should clip its contents and allow
        // scrolling rather than expanding to contain them.
        Clay_ScrollElementConfig scroll;
        // Controls settings related to element borders, and will generate
        // BORDER render commands.
        Clay_BorderElementConfig border;
        // A pointer that will be transparently passed through to resulting
        // render commands.
        void* userData;

        ClayElementDeclarationPartial(Clay_ElementDeclaration decl) {
            this->layout          = decl.layout;
            this->backgroundColor = decl.backgroundColor;
            this->cornerRadius    = decl.cornerRadius;
            this->image           = decl.image;
            this->floating        = decl.floating;
            this->custom          = decl.custom;
            this->scroll          = decl.scroll;
            this->border          = decl.border;
            this->userData        = decl.userData;
        }
    };

    void render_simple_button(std::string_view text, soil::Color color) noexcept;

    static inline Clay_String to_clay(const std::string_view string) noexcept {
        return Clay_String{
            .length = (i32)string.length(),
            .chars  = string.data(),
        };
    }

    static inline Clay_String to_clay_last(const std::string_view string) noexcept {
        // char* ptr = (char*)malloc(string.length());

        char* ptr = (char*)CLAY_COPY_BUFFER.allocate(string.length());

        memcpy(ptr, string.data(), string.length());

        return Clay_String{.length = (i32)string.length(), .chars = ptr};
    }

    void
    new_element(Clay_ElementDeclaration declaration, std::function<void()>&& inner) noexcept;

    // Ripped *right* from clay

    [[nodiscard]] static constexpr inline Clay_ElementId
    hash_string(const Clay_String key, const u32 offset = 0, const u32 seed = 0) noexcept {
        uint32_t hash = 0;
        uint32_t base = seed;

        for (int32_t i = 0; i < key.length; i++) {
            base += key.chars[i];
            base += (base << 10);
            base ^= (base >> 6);
        }
        hash  = base;
        hash += offset;
        hash += (hash << 10);
        hash ^= (hash >> 6);

        hash += (hash << 3);
        base += (base << 3);
        hash ^= (hash >> 11);
        base ^= (base >> 11);
        hash += (hash << 15);
        base += (base << 15);
        return {
            .id = hash + 1, .offset = offset, .baseId = base + 1, .stringId = key
        }; // Reserve the hash result of zero as "null id"
    }

    [[nodiscard]] static constexpr inline Clay_ElementId
    hash_string(const std::string_view string) noexcept {
        return hash_string(to_clay(string), 0, 0);
    }

    struct ButtonConfig {
        soil::Color text_color;
        soil::Color background_color;
        f32         hover_light_amount = 0.2;
        f32         rounding           = 10;
        u16         text_padding       = 8;
        u16         font_size          = 20;
        soil::Vec2i min_size           = {0, 0};

        [[nodiscard]] bool render_button(
            std::string_view text, Clay_ElementId id,
            std::optional<std::function<void(const ButtonConfig&, std::string_view)>>&&
                 callback = std::nullopt,
            bool bare     = false
        ) const noexcept;

        [[nodiscard]] Clay_ElementDeclaration
        create_decleration(bool hovered, const Clay_ElementId& id) const noexcept;

        [[nodiscard]] bool inline is_hovered(const Clay_ElementId& id) const noexcept {
            bool hovered = Clay_PointerOver(id);
            return hovered;
        }
    };

} // namespace clay_extension
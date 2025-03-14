#pragma once
#include "../../clay/components.hpp"
#include "../editor_config.hpp"
#include <functional>
#include <string>

namespace soil {
    
    class BasicButton : public clay_extension::BaseButton<BasicButton, EditorVisualConfig> {
    public:
        using Parent = clay_extension::BaseButton<BasicButton, EditorVisualConfig>;

        BasicButton(EditorVisualConfig& config, std::string&& text, const std::string& id, std::function<void()> callback) noexcept : Parent(
            clay_extension::ButtonConfig{
                .text_color = config.normal_text_color,
                .background_color = config.header_button_color,
                .font_size = 17
            },
            id, std::move(text)
        ), callback(callback) {}

        void inline on_click() {
            this->callback();
        }

    private:
        std::function<void()> callback;

    };
}


//
// Created by Akashi on 02/03/25.
//

#ifndef APPLICATION_HPP
#define APPLICATION_HPP
#include "../clay/components.hpp"
#include "editor_config.hpp"

namespace soil {
using namespace clay_extension;
class Application {
public:
    Application() noexcept;

    void render() noexcept;

private:
    void render_head_bar() noexcept;

private:
    class SettingsButton
        : public BaseButton<SettingsButton, EditorVisualConfig> {
    public:
        using Parent = BaseButton<SettingsButton, EditorVisualConfig>;

        SettingsButton(EditorVisualConfig &conf) noexcept
            : Parent(
                  ButtonConfig{
                      .text_color       = conf.normal_text_color,
                      .background_color = conf.header_button_color,
                      .font_size        = 17
                  },
                  "Settings", "Settings"
              ) {}
    };

private:
    using HeaderType = HeaderBar<EditorVisualConfig, SettingsButton>;

    EditorVisualConfig visual_config{};
    HeaderType         header;
};

} // namespace soil

#endif // APPLICATION_HPP

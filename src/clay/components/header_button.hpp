
#pragma once
#include "../clay_binding.hpp"
#include "../templates.hpp"
#include "header.hpp"
#include <optional>

SIMPLE_OPTIONAL_FUNCTION(on_click, OnClick);

namespace clay_extension {
    template <typename BaseClass, typename ConfigType> class BaseButton {
    public:
        void render(ConfigType &, const RenderInformation &) noexcept {
            const auto clicked = this->config.render_button(
                this->render_text, this->ele_id, std::nullopt, true
            );

            if (clicked)
                OnClickCaller<BaseClass>::on_click(
                    static_cast<BaseClass &>(*this)
                );
        }

        std::string id() const noexcept { return this->render_id; }

        std::optional<ClayElementDeclarationPartial>
        custom_declaration(const RenderInformation &render) const noexcept {
            const auto info = this->config.create_decleration(
                render.is_hovered, this->ele_id
            );
            return ClayElementDeclarationPartial{info};
        }

        BaseButton(
            ButtonConfig config, const std::string &id,
            std::string &&render_text
        ) noexcept
            : config(config), render_id(id),
              render_text(std::move(render_text)) {
            this->ele_id = hash_string(id);
        }

    public:
        ButtonConfig config;
        std::string  render_text{};

    private:
        std::string    render_id;
        Clay_ElementId ele_id;
    };
} // namespace clay_extension
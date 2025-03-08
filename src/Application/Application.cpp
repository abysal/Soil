//
// Created by Akashi on 02/03/25.
//

#include "Application.hpp"

#include <print>
#include <stdexcept>
#include <utility>

#include "../clay/clay_binding.hpp"

using namespace clay_extension;

namespace soil {
    Application::Application() noexcept
        : header(HeaderType(
              {
                  .id = hash_string("HeaderBar"),
                  .layout =
                      {.sizing =
                           {.width  = CLAY_SIZING_GROW(0),
                            .height = {f32(std::to_underlying(
                                this->visual_config.bar_height
                            ))}},
                       .padding         = CLAY_PADDING_ALL(8),
                       .childGap        = 16,
                       .childAlignment  = {.y = CLAY_ALIGN_Y_CENTER},
                       .layoutDirection = CLAY_LEFT_TO_RIGHT},
                  .backgroundColor = this->visual_config.header_color_base,
              },
              HeaderType::ElementConfig{
                  .background_color = this->visual_config.header_color_base,
              },
              SettingsButton(this->visual_config)
          )) {}

    void Application::render() noexcept {
        constexpr Clay_Sizing layout = {
            .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)
        };
        CLAY({
            .id = hash_string("Holder"),
            .layout =
                {
                    .sizing          = layout,
                    .padding         = Clay_Padding(0, 0, 0, 0),
                    .childGap        = 16,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            .backgroundColor = this->visual_config.base_color,
        }) {

            this->render_head_bar();
        }
    }

    void Application::render_head_bar() noexcept {

        const auto bar_height =
            f32(std::to_underlying(this->visual_config.bar_height));

        new_element(
            {.id = hash_string("Header Holder"),
             .layout =
                 {.layoutDirection = CLAY_TOP_TO_BOTTOM,
                  .sizing =
                      {.width = CLAY_SIZING_GROW(), .height = {bar_height}}}},
            [&] {
                this->header.render(this->visual_config);

                new_element(
                    {.id = hash_string("Underline"),
                     .layout =
                         {.sizing =
                              {
                                  .width  = CLAY_SIZING_GROW(),
                                  .height = 6,
                              }},
                     .backgroundColor = this->visual_config.header_stripe_color
                    },
                    [&] {}
                );
            }
        );
    }

} // namespace soil

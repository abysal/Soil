#include "application_sidebar.hpp"
#include "clay/clay_binding.hpp"

namespace soil {
    using namespace clay_extension;
    void ApplicationSidebar::render(EditorVisualConfig& config) noexcept {

        Clay_Sizing size = {
            .width  = CLAY_SIZING_FIXED(300),
            .height = CLAY_SIZING_GROW(),
        };

        new_element(
            Clay_ElementDeclaration{
                .id = hash_string("Sidebar"),
                .layout =
                    {
                        .sizing  = size,
                        .padding = {4, 4, 4, 4},
                    },
                .backgroundColor = config.sidebar_background_color,

            },
            [&] {
                const auto folder_view_id = hash_string("FolderView");

                this->tree.render(
                    {.id               = folder_view_id,
                     .background_color = config.sidebar_background_color,
                     .outline_color    = config.accent_color,
                     .text_color       = config.normal_text_color}
                );
            }
        );
    }
} // namespace soil
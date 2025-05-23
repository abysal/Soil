//
// Created by Akashi on 02/03/25.
//

#include "application.hpp"

#include <filesystem>
#include <memory>
#include <print>
#include <utility>

#include "../clay/clay_binding.hpp"
#include "Application/application_sidebar.hpp"
#include "Application/components/folder_tree.hpp"
#include "Application/project/project.hpp"
#include "DirectXMagic/dx_creation.hpp"
#include "clay/clay_binding.hpp"
#include "clay/templates.hpp"
#include "components/simple_button.hpp"
#include "types.hpp"
#include "windows/injector.hpp"
#include <print>
#include <tfd/tinyfiledialogs.h>

using namespace clay_extension;

namespace soil {
    Application::Application() noexcept
        : header(HeaderType(
              {
                  .id = hash_string("HeaderBar"),
                  .layout =
                      {.sizing =
                           {.width  = CLAY_SIZING_GROW(0),
                            .height = {f32(std::to_underlying(this->visual_config.bar_height))}
                           },
                       .padding         = CLAY_PADDING_ALL(8),
                       .childGap        = 16,
                       .childAlignment  = {.y = CLAY_ALIGN_Y_CENTER},
                       .layoutDirection = CLAY_LEFT_TO_RIGHT},
                  .backgroundColor = this->visual_config.header_color_base,
              },
              HeaderType::ElementConfig{
                  .background_color = this->visual_config.header_color_base,
              },
              BasicButton(
                  this->visual_config, "Open Project \u003D", "OpenProject",
                  [&] {
                      const char* selected =
                          tinyfd_selectFolderDialog("Project Folder", nullptr);

                      if (!selected) {
                          return; // Just means they didnt select anything
                      }

                      this->handle_project_change(selected);
                  }
              ),
              BasicButton(this->visual_config, "Settings", "Settings", [&] {})
          )) {
        this->d3d12_ctx = std::make_unique<D3D::D3D12>(std::move(D3D::D3D12{}));
        Clay_SetDebugModeEnabled(true);
    }

    void Application::render() noexcept {
        if (this->dx_12_talker.ipc_created()) {
            this->dx_12_talker.get_ipc().consumed_frame = true;
        }

        constexpr Clay_Sizing layout = {
            .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0)
        };
        CLAY({
            .id = hash_string("Holder"),
            .layout =
                {
                    .sizing          = layout,
                    .padding         = Clay_Padding(0, 0, 0, 0),
                    .childGap        = 0,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            .backgroundColor = this->visual_config.base_color,
        }) {
            this->render_head_bar();
            new_element(
                {.layout.layoutDirection = CLAY_LEFT_TO_RIGHT,
                 .layout.sizing          = {CLAY_SIZING_GROW(), CLAY_SIZING_GROW()}},
                [&] {
                    if (this->sidebar) {
                        this->sidebar->render(this->visual_config);
                    }
                }
            );
        }
    }

    void Application::render_head_bar() noexcept {

        const auto bar_height = f32(std::to_underlying(this->visual_config.bar_height));

        new_element(
            {.id = hash_string("Header Holder"),
             .layout =
                 {
                     .sizing          = {.width = CLAY_SIZING_GROW(), .height = {bar_height}},
                     .layoutDirection = CLAY_TOP_TO_BOTTOM,
                 }},
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
                     .backgroundColor = this->visual_config.header_stripe_color},
                    [&] {}
                );
            }
        );
    }

    void Application::handle_project_change(const char* new_path) noexcept {
        std::filesystem::path path{new_path};

        Project project{std::move(path)};

        project.tree().lock()->deep_index();

        this->project = std::make_unique<Project>(std::forward<Project>(project));

        if (this->sidebar) {
            this->sidebar->tree.reset_ptr(this->project->tree());
        } else {
            FolderTree tree{this->project->tree()};

            this->sidebar =
                std::make_unique<ApplicationSidebar>(ApplicationSidebar{std::move(tree)});
        }
    }

} // namespace soil

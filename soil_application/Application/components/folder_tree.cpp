#include "./folder_tree.hpp"
#include "clay/templates.hpp"
#include "types.hpp"
#include <clay/clay_binding.hpp>
#include <print>

using namespace soil;

void FolderTree::render(const FolderTreeConfig &config) noexcept {
    if (this->tree.expired()) {
        return;
    }

    auto ptr = this->tree.lock();

    auto &root = ptr->root();

    usize indent_level = 0;

    using namespace clay_extension;

    usize ele_count = 0;

    const std::function<void(FileNode &)> render_layer = std::function([&](FileNode &base) {
        for (const auto &node : base.as_folder().children) {
            usize offset = indent_level * config.indent_per_level;

            new_element({.layout = {.sizing = {CLAY_SIZING_GROW()}}}, [&] {
                Color color = Clay_Hovered() ? config.background_color *
                                                   (1. + config.on_hover_lighten_amount)
                                             : config.background_color;

                new_element(
                    {
                        .layout          = {.padding = {0, 4, 0, 0}},
                        .backgroundColor = color,
                    },
                    [&] {
                        new_element(
                            {.layout =
                                 {.sizing =
                                      {.width = (f32)offset, .height = (f32)config.node_height}}
                            },
                            [&] {}
                        );

                        auto        name = node->full_name();
                        Clay_String text = to_clay_last(name);

                        Clay_TextElementConfig text_config = {
                            .textColor = config.text_color,
                            .fontSize  = config.font_size,
                        };

                        CLAY_TEXT(text, Clay__StoreTextElementConfig(text_config));
                    }
                );

                new_element(
                    {.layout          = {.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}},
                     .backgroundColor = color},
                    [&] {}
                );
            });
            if (node->is_folder()) {
                indent_level++;
                render_layer(*node.get());
                indent_level--;
            }
        }
    });

    new_element(
        Clay_ElementDeclaration{
            .id = config.id,
            .layout =
                {
                    .sizing          = {CLAY_SIZING_FIT(), CLAY_SIZING_GROW()},
                    .padding         = {4, 4, 4, 4},
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                },
            .backgroundColor = config.background_color,
            .scroll          = {true, true},
            //.border          = {.color = config.outline_color}},
        },
        [&] {
            // root will always be a folder, if it isn't there is major shit happening
            render_layer(root);
        }
    );
}
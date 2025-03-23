#include "./folder_tree.hpp"
#include "clay/templates.hpp"
#include "raylib.h"
#include "types.hpp"
#include <cassert>
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

            const bool deep_render =
                node->is_folder() && this->state.enabled_folders.contains(node->gash());

            new_element({.layout = {.sizing = {CLAY_SIZING_GROW()}}}, [&] {
                const auto hovered = Clay_Hovered();
                Color      color =
                    hovered ? config.background_color * (1. + config.on_hover_lighten_amount)
                                 : config.background_color;

                new_element(
                    {
                        .layout =
                            {.padding = {0, 4, 0, 0},
                             .sizing  = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}},
                        .backgroundColor = color,
                    },
                    [&] {
                        if (node->is_folder()) {

                            auto txt_cfg = text_config(
                                {.fontSize = config.font_size, .textColor = {255, 255, 255, 255}
                                }
                            );

                            if (this->state.show(node->gash())) {
                                CLAY_TEXT(CLAY_STRING("-"), txt_cfg.get());
                            } else {
                                CLAY_TEXT(CLAY_STRING("+"), txt_cfg.get());
                            }
                        }

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

                if (hovered) {
                    this->handle_hover(owner_id(), node->gash());
                }
            });

            if (deep_render) {
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

void FolderTree::handle_hover(Clay_ElementId id, usize global_hash) noexcept {
    assert(!this->tree.expired()
    ); // If this triggers this function was somehow called outside of the renderer

    if (!IsMouseButtonPressed(0)) {
        return; // For now we only handle clicks
    }

    const auto ptr = this->tree.lock();

    const auto node = ptr->get_node_from_hash(global_hash);

    if (node->is_file()) {
        return; // TODO: Handle active file switching
    }

    if (this->state.enabled_folders.contains(node->gash())) {
        this->state.enabled_folders.erase(node->gash());
    } else {
        this->state.enabled_folders.insert(node->gash());
    }
}
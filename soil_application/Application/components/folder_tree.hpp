#pragma once
#include "Application/project/filesystem_tree.hpp"
#include "types.hpp"
#include <memory>
#include <unordered_set>

namespace soil {

    struct FolderTreeConfig {
        usize          indent_per_level{20};
        usize          node_height{20};
        usize          per_element_gap{1};
        f32            on_hover_lighten_amount{.2};
        u16            font_size{15};
        Clay_ElementId id;
        Color          background_color;
        Color          outline_color;
        Color          text_color;
    };

    class FolderTree {
    private:
        struct State {
            std::unordered_set<usize> enabled_folders{};

            bool show(usize hash) const noexcept {
                return this->enabled_folders.contains(hash);
            }
        };

    public:
        FolderTree(std::weak_ptr<FilesystemTree> ptr) noexcept : tree(ptr) {}

        void render(const FolderTreeConfig &config) noexcept;

        void reset_ptr(std::weak_ptr<FilesystemTree> ptr) noexcept {
            this->tree.reset();
            this->tree  = ptr;
            this->state = State{};
        }

    private:
        void handle_hover(Clay_ElementId id, usize global_hash) noexcept;

    private:
        std::weak_ptr<FilesystemTree> tree{};
        State                         state{};
    };
} // namespace soil
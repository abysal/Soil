#pragma once
#include "components/folder_tree.hpp"
#include "./editor_config.hpp"

namespace soil {
    class ApplicationSidebar {
    public:
        ApplicationSidebar(FolderTree&& tree) noexcept : tree(tree) {};

        void render(EditorVisualConfig& config) noexcept;

    private:
        friend class Application;
        FolderTree tree;
    };
} // namespace soil

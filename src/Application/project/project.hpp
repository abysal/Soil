#pragma once

#include "Application/project/filesystem_tree.hpp"
#include <filesystem>
#include <memory>

// TODO: Implement Filesystem refreshing

namespace soil {
    class Project {
    public:
        Project(std::filesystem::path &&project_path)
            : project_path(project_path),
              fs_tree(
                  std::make_shared<FilesystemTree>(std::move(FilesystemTree{std::filesystem::path{this->project_path}}))
              ) {};

        const std::filesystem::path &proj_path() const noexcept { return this->project_path; }

        std::weak_ptr<FilesystemTree> tree() noexcept { return this->fs_tree; }

    private:
    private:
        std::filesystem::path           project_path;
        std::shared_ptr<FilesystemTree> fs_tree{nullptr};
    };
} // namespace soil
#pragma once
#include "Application/memory/pointer.hpp"
#include "clay/templates.hpp"
#include "types.hpp"
#include <cassert>
#include <filesystem>
#include <format>
#include <functional>
#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

namespace soil {

    class FileNode {
    public:
        struct File {
            std::string extension{};
        };

        struct Folder {
            std::vector<std::unique_ptr<FileNode>> children{};
        };

        constexpr static FileNode file(
            std::observer_ptr<FileNode> parent, std::string &&name, std::string &&extension = {}
        ) noexcept {
            FileNode node{};

            node.name       = std::move(name);
            node.parent     = parent;
            node.inner_file = File{extension};

            node.name_hash = std::hash<std::string>{}(node.name);
            hash_combine(node.name_hash, std::hash<std::string>{}(node.file_extension()));

            return node;
        }

        template <typename Self> constexpr inline auto &as_folder(this Self &self) noexcept {
            assert(self.is_folder());

            return std::get<Folder>(self.inner_file);
        }

        constexpr usize index_from_name_hash(usize name_hash) const noexcept {
            const Folder &folder = this->as_folder();

            usize idx = 0;
            for (auto &file : folder.children) {
                if (file->name_hash == name_hash) {
                    return idx;
                }
                idx++;
            }

            return (usize)-1;
        }

        constexpr std::observer_ptr<FileNode> node_from_index(usize index) noexcept {
            return std::make_observer(this->as_folder().children[index].get());
        }

        constexpr inline usize last_pushed_index() const noexcept {
            return this->as_folder().children.size() - 1;
        }

        constexpr static FileNode
        folder(std::observer_ptr<FileNode> parent, std::string &&name) noexcept {
            FileNode node{};

            node.name       = std::move(name);
            node.parent     = parent;
            node.inner_file = Folder{};
            node.name_hash  = std::hash<std::string>{}(node.name);
            return node;
        }

        constexpr inline bool is_file() const noexcept {
            return std::holds_alternative<File>(this->inner_file);
        }

        constexpr inline bool is_folder() const noexcept { return !this->is_file(); }

        constexpr inline const std::string &file_extension() const noexcept {
            assert(this->is_file());

            return std::get<File>(this->inner_file).extension;
        }

        constexpr inline const std::string &base_name() const noexcept { return this->name; }

        constexpr inline std::string full_name() const noexcept {
            if (this->is_file()) {
                return std::format("{}{}", this->name, this->file_extension());
            } else {
                return this->name;
            }
        }

        constexpr inline bool name_changed(usize old_hash) const noexcept {
            return this->name_hash != old_hash;
        }

        constexpr inline void set_name(std::string &&name) noexcept {
            if (this->is_file()) {
                this->name = name;

                this->name_hash = std::hash<std::string>{}(this->name);
                hash_combine(this->name_hash, std::hash<std::string>{}(this->file_extension()));
            } else {
                this->name = name;

                this->name_hash = std::hash<std::string>{}(this->name);
            }
        }

        constexpr inline void set_extension(std::string &&ext) noexcept {
            assert(this->is_file());

            std::get<File>(this->inner_file).extension = ext;

            this->name_hash = std::hash<std::string>{}(this->name);
            hash_combine(this->name_hash, std::hash<std::string>{}(this->file_extension()));
        }

        constexpr inline usize hash() const noexcept { return this->name_hash; }

        constexpr inline void new_member(FileNode &&member) noexcept {
            assert(this->is_folder());

            std::get<Folder>(this->inner_file)
                .children.emplace_back(std::make_unique<FileNode>(std::forward<FileNode>(member)
                ));
        }

        constexpr std::observer_ptr<FileNode> get_parent() noexcept { return this->parent; }

        constexpr usize gash() const noexcept { return this->global_hash; }

    private:
        friend class FilesystemTree;
        usize                                      name_hash{(usize)-1};
        usize                                      global_hash{(usize)-1};
        std::observer_ptr<FileNode>                parent{nullptr};
        std::string                                name{};
        std::variant<std::monostate, Folder, File> inner_file{};
    };

    class FilesystemTree {
    public:
        FilesystemTree(FilesystemTree &&) = default;

        FilesystemTree(const FilesystemTree &) = delete;

        FilesystemTree(std::filesystem::path &&root_path) noexcept : root_path(root_path) {
            this->index();
        };

        void index();
        void deep_index() noexcept;

        std::observer_ptr<FileNode> get_node_from_hash(usize hash) noexcept {
            try {
                return this->global_hash_loopup.at(hash);
            } catch (...) {
                return nullptr;
            }
        }

        FileNode &root() noexcept { return this->root_node; }

    private:
        std::filesystem::path root_path{};
        FileNode              root_node = FileNode::folder(nullptr, "");
        std::unordered_map<usize, std::observer_ptr<FileNode>> global_hash_loopup{};
    };
} // namespace soil
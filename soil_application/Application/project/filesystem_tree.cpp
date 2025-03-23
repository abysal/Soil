#include "./filesystem_tree.hpp"
#include "Application/memory/pointer.hpp"
#include "clay/templates.hpp"
#include <Application/memory/alloc_wrapper.hpp>
#include <Application/memory/fallback_allocator.hpp>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <memory_resource>
#include <ostream>
#include <type_traits>
#include <utility>
#include <vector>

namespace soil {

    void FilesystemTree::deep_index() noexcept {
        namespace pmr = std::pmr;
        using Alloc   = FallBackAllocator;
        alignas(std::max_align_t) std::array<u8, 8192> allocation{};

        std::span<u8> memory = allocation;

        auto backup_alloc = std::pmr::new_delete_resource();

        Alloc base_allocator{
            std::observer_ptr<std::pmr::memory_resource>(backup_alloc), memory
        };

        pmr::vector<std::observer_ptr<FileNode>> nodes{&base_allocator};

        const auto compute_hash = [&](std::observer_ptr<FileNode> node) {
            usize end_hash = node->hash();

            std::observer_ptr<FileNode> ptr = node->get_parent();

            while (ptr) {
                hash_combine(end_hash, ptr->hash());
                ptr = ptr->get_parent();
            }

            while (this->global_hash_loopup.contains(end_hash) &&
                   this->global_hash_loopup.at(end_hash) != node) {
                end_hash++;
            }

            node->global_hash = end_hash;

            this->global_hash_loopup[end_hash] = node;
        };

        nodes.push_back(&this->root_node);

        while (!nodes.empty()) {
            pmr::vector<std::observer_ptr<FileNode>> next_iter{&base_allocator};

            for (auto ptr : nodes) {
                compute_hash(ptr);

                if (ptr->is_file()) {
                    continue;
                }

                for (auto &p : ptr->as_folder().children) {
                    next_iter.push_back(p.get());
                }
            }

            std::swap(nodes, next_iter);
        }
    }

    void FilesystemTree::index() {
        using Alloc = FallBackAllocator;

        constexpr usize allocation_size = (usize)134217728;
        std::size_t     alignment       = std::alignment_of_v<std::max_align_t>;
        auto            deleter         = [](u8 *ptr) {
            soil::aligned_free_wrapper(ptr);
            std::println("Freeing Slab");
        };

        std::unique_ptr<u8[], decltype(deleter)> use_memory(
            static_cast<u8 *>(soil::aligned_alloc_wrapper(alignment, allocation_size)), deleter
        );

        std::span<u8> memory = std::span<u8>{use_memory.get(), allocation_size};

        auto backup_alloc = std::pmr::new_delete_resource();

        Alloc base_allocator{
            std::observer_ptr<std::pmr::memory_resource>(backup_alloc), memory
        };

        namespace pmr = std::pmr;
        namespace fs  = std::filesystem;

        struct FolderVisiter {
            const std::observer_ptr<FileNode> insert_target{nullptr};
            const pmr::string                 folder_location{};
        };

        // Using PMR for faster temp allocations
        pmr::vector<FolderVisiter> too_visit{&base_allocator};

        std::observer_ptr<FileNode> active_node = std::make_observer(&this->root_node);

        const auto single_layer = [&](std::observer_ptr<FileNode> parent_node,
                                      pmr::vector<FolderVisiter> &out_vector,
                                      const fs::path             &loop_path) {
            fs::directory_iterator root_iter{
                loop_path, fs::directory_options::skip_permission_denied |
                               fs::directory_options::follow_directory_symlink
            };
            for (const auto &x : root_iter) {
                try {
                    if (x.is_regular_file()) {
                        const auto &path = x.path();

                        std::string name = path.filename().string();
                        std::string ext  = path.extension().string();

                        if (!ext.empty()) {
                            name = name.substr(0, name.size() - ext.length());
                        }

                        auto node =
                            FileNode::file(parent_node, std::move(name), std::move(ext));

                        parent_node->new_member(std::move(node));
                    } else if (x.is_directory()) {
                        std::string name = x.path().filename().string();

                        auto node = FileNode::folder(parent_node, std::move(name));

                        parent_node->new_member(std::move(node));

                        pmr::string full_path{x.path().string(), &base_allocator};

                        const usize folder_index = parent_node->last_pushed_index();
                        auto        parent       = parent_node->node_from_index(folder_index);

                        out_vector.emplace_back(
                            FolderVisiter{.insert_target = parent, .folder_location = full_path}
                        );
                    }
                } catch (std::filesystem::filesystem_error err) {
                    std::println("{}", err.what());
                } catch (...) {
                    // TODO: Handle UTF8 PROPERLY
                    std::println("Unknown Error");
                }
            }
        };

        single_layer(active_node, too_visit, this->root_path);

        while (!too_visit.empty()) {
            pmr::vector<FolderVisiter> next_iteration{&base_allocator};

            for (auto &section : too_visit) {
                fs::path pth{section.folder_location};

                single_layer(section.insert_target, next_iteration, pth);
            }

            std::swap(too_visit, next_iteration);
        }
    }
} // namespace soil
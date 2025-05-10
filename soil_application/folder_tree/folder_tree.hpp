#pragma once
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <gsl/pointers>
#include <limits>
#include <optional>
#include <string>
#include <vector>

namespace Rml {
    class Context;
    class Element;
    class ElementDocument;
} // namespace Rml

namespace soil {
    class FSTree {
    public:
        struct FileTreeNode {

            constexpr static size_t NO_PARENT = std::numeric_limits<size_t>::max();

            std::string name          = {};
            bool        is_visible    = true;
            uint32_t    depth         = {0};
            size_t      parent        = NO_PARENT;
            size_t      raw_arr_index = 0;

            struct FolderInfo {
                bool                collapsed = false;
                std::vector<size_t> children  = {};
            };

            std::optional<FolderInfo> folder_info{
            }; // Only populated for folders, if this is empty. Its a folder

            bool is_folder() { return this->folder_info.has_value(); }

            bool is_collapsed() {
                assert(this->is_folder());

                return this->folder_info.value().collapsed;
            }

            std::vector<size_t>& children() {
                assert(this->is_folder());

                return this->folder_info.value().children;
            }
        };

    public:
        FSTree(gsl::not_null<class Rml::Context*> owner_ctx, const std::filesystem::path& folder_to_scan, struct SoilSettings&);

        void
        render_in(class Rml::Element& parent_div, class Rml::ElementDocument& owner_document);

        void
        render_in_internal(class Rml::Element& parent, uint32_t depth, size_t element_index);

        class Rml::Element&
        create_file(std::string&& name, uint32_t depth, class Rml::Element& parent);

        class Rml::Element& create_folder(
            std::string&& name, uint32_t depth, bool expanded, class Rml::Element& parent
        );

    private:
        void bind_data();

        void scan_tree();

        void scan_tree_inner(const std::filesystem::path& dir, size_t parent, uint32_t depth);

    private:
        gsl::not_null<class Rml::Context*> owner;
        struct SoilSettings&               settings;
        std::filesystem::path              scan_root  = {};
        std::vector<FileTreeNode>          filesystem = {};
    };
} // namespace soil
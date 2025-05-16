#pragma once
#include <RmlUi/Core/EventListener.h>
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <functional>
#include <gsl/pointers>
#include <limits>
#include <optional>
#include <string>
#include <vector>

namespace Rml {
    class Context;
    class Element;
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
                bool                collapsed = true;
                std::vector<size_t> children  = {};
            };

            std::optional<FolderInfo> folder_info{
            }; // Only populated for folders, if this is empty. Its a folder

            bool is_folder_script() { return this->folder_info.has_value(); }
            bool is_folder() const { return this->folder_info.has_value(); }

            bool is_collapsed_script() {
                assert(this->is_folder());

                return this->folder_info.value().collapsed;
            }

            bool is_collapsed() const {
                assert(this->is_folder());

                return this->folder_info.value().collapsed;
            }

            std::vector<size_t>& children() {
                assert(this->is_folder());

                return this->folder_info.value().children;
            }
        };

    public:
        FSTree(const FSTree&)            = delete;
        FSTree(FSTree&&)                 = default;
        FSTree& operator=(const FSTree&) = delete;
        FSTree& operator=(FSTree&&)      = default;
        FSTree(gsl::not_null<class Rml::Context*> owner_ctx, const std::filesystem::path& folder_to_scan, struct SoilSettings&);

        void render_in(class Rml::Element& parent_div);
        void scan_tree();

    private:
        void
        render_in_internal(class Rml::Element& parent, uint32_t depth, size_t element_index);

        class Rml::Element&
        create_file(const std::string& name, uint32_t depth, class Rml::Element& parent) const;

        class Rml::Element& create_folder(
            const std::string& name, uint32_t depth, bool expanded, class Rml::Element& parent
        ) const;

        void bind_data();

        void scan_tree_inner(const std::filesystem::path& dir, size_t parent, uint32_t depth);

    private:
        struct Handler : public Rml::EventListener {
            size_t  element_index{0};
            FSTree* owner{nullptr};

            Handler(gsl::not_null<FSTree*> owner, size_t element)
                : element_index(element), owner(owner) {}
            Handler() = default;

            void ProcessEvent(class Rml::Event& /*dont care*/) override;

            // Clean up
            void OnDetach(class Rml::Element*) override { delete this; }
        };

    private:
        gsl::not_null<class Rml::Context*>          owner;
        std::reference_wrapper<struct SoilSettings> settings;
        std::filesystem::path                       scan_root  = {};
        std::vector<FileTreeNode>                   filesystem = {};

        friend struct Handler;
    };
} // namespace soil
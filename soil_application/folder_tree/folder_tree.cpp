#include "./folder_tree.hpp"
#include "flag_manager/flag_manager.hpp"
#include "settings.hpp"
#include <RmlUi/Core/Box.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/ElementInstancer.h>
#include <RmlUi/Core/Factory.h>
#include <RmlUi/Core/ID.h>
#include <RmlUi/Core/Log.h>
#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Variant.h>
#include <filesystem>
#include <gsl/pointers>
#include <stdexcept>

namespace soil {
    constexpr float ExpanderSize = 20.F;

    FSTree::FSTree(
        gsl::not_null<Rml::Context*> owner_ctx, const std::filesystem::path& root,
        SoilSettings& settings
    )
        : owner(owner_ctx), settings(settings), scan_root(root) {
        this->bind_data();
    }

    void
    FSTree::scan_tree_inner(const std::filesystem::path& dir, size_t parent, uint32_t depth) {
        auto iter = std::filesystem::directory_iterator{dir};

        const auto our_depth = depth + 1;

        for (const auto& info : iter) {
            const auto our_index = filesystem.size();

            try {
                // TODO: Switch to a custom Wide char to UTF-8 on windows
                FileTreeNode node = {
                    .name          = info.path().filename().string(),
                    .is_visible    = true,
                    .depth         = our_depth,
                    .parent        = parent,
                    .raw_arr_index = our_index
                };

                this->filesystem.emplace_back(node);

                // Tells our parent we exist
                this->filesystem[parent].children().push_back(our_index);

                if (info.status().type() != std::filesystem::file_type::directory) {
                    continue;
                }

                this->filesystem[our_index].folder_info = FileTreeNode::FolderInfo{};

                this->scan_tree_inner(info.path(), our_index, our_depth);
            } catch (std::runtime_error& err) {
                Rml::Log::Message(Rml::Log::LT_ERROR, "%s", err.what());
            }
        }
    };

    void FSTree::Handler::ProcessEvent(class Rml::Event& /*dont care*/) {
        assert(this->owner->filesystem[element_index].is_folder());

        auto& info = this->owner->filesystem[element_index].folder_info.value();

        info.collapsed = !info.collapsed;

        FlagManager::flag_manager().set_rebuild_tree();
    }

    void
    FSTree::render_in_internal(Rml::Element& parent, uint32_t depth, size_t element_index) {

        if (const auto& file = this->filesystem[element_index]; file.is_folder()) {

            auto& folder = this->create_folder(file.name, depth, file.is_collapsed(), parent);
            const auto& [collapsed, children] = file.folder_info.value();

            const auto file_index = file.raw_arr_index;
            auto*      fs_tree    = this;

            folder.AddEventListener(Rml::EventId::Click, new Handler{fs_tree, file_index});

            if (file.is_collapsed()) {
                return;
            }

            for (const auto& child : children) {
                this->render_in_internal(parent, depth + 1, child);
            }

        } else {
            (void)this->create_file(file.name, depth, parent);
        }
    }

    class Rml::Element& FSTree::create_folder(
        const std::string& name, uint32_t depth, bool expanded, class Rml::Element& parent
    ) const {

        Rml::XMLAttributes attribs = {{"class", Rml::Variant{"file_row lighter_less"}}};
        auto button = Rml::Factory::InstanceElement(&parent, "*", "button", attribs);

        auto* button_ptr = parent.AppendChild(std::move(button));

        button_ptr->AppendChild(
            Rml::Factory::InstanceElement(
                button_ptr, "*", "div",
                {{"class", Rml::Variant{"file_filler"}},
                 {"data-style-width",
                  Rml::Variant{std::format("str_cat(per_layer_gap * {}, 'dp')", depth)}}}
            )
        );

        button_ptr->AppendChild([&] {
            auto ele = Rml::Factory::InstanceElement(
                button_ptr, "*", "div", {{"class", Rml::Variant{"folder_filler"}}}
            );

            Rml::Factory::InstanceElementText(
                ele.get(), std::format("<p>{}</p>", expanded ? "+" : "-")
            );

            return ele;
        }());

        button_ptr->AppendChild([&] {
            auto ele = Rml::Factory::InstanceElement(button_ptr, "*", "span", {});

            Rml::Factory::InstanceElementText(ele.get(), std::format("<p>{}</p>", name));

            return ele;
        }());

        return *button_ptr;
    }

    Rml::Element&
    FSTree::create_file(const std::string& name, uint32_t depth, Rml::Element& parent) const {
        Rml::XMLAttributes attribs = {{"class", Rml::Variant{"file_row lighter_less"}}};
        auto button = Rml::Factory::InstanceElement(&parent, "*", "button", attribs);

        auto* button_ptr = parent.AppendChild(std::move(button));

        button_ptr->AppendChild(
            Rml::Factory::InstanceElement(
                button_ptr, "*", "div",
                {{"class", Rml::Variant{"file_filler"}},
                 {"data-style-width",
                  Rml::Variant{
                      std::format("str_cat(per_layer_gap * {} + {}, 'dp')", depth, ExpanderSize)
                  }}}
            )
        );

        button_ptr->AppendChild([&] {
            auto ele = Rml::Factory::InstanceElement(button_ptr, "*", "span", {});

            Rml::Factory::InstanceElementText(ele.get(), std::format("<p>{}</p>", name));

            return ele;
        }());

        return *button_ptr;
    }

    void FSTree::render_in(Rml::Element& parent) {

        for (const auto& root = this->filesystem[0];
             const auto& child : root.folder_info->children) {
            this->render_in_internal(parent, 0, child);
        }
    }

    void FSTree::scan_tree() {
        FileTreeNode root = {
            .name        = "",
            .is_visible  = true,
            .depth       = 0,
            .folder_info = FileTreeNode::FolderInfo{.collapsed = false, .children = {}},
        };

        this->filesystem.emplace_back(std::move(root));

        this->scan_tree_inner(this->scan_root, 0, 0);
    }

    void FSTree::bind_data() {}

} // namespace soil
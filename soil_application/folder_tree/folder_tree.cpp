#include "./folder_tree.hpp"
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
    // TODO:
    // Implement the generation of the folder tree
    // Implement the handling of showing and hiding

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

            if (!node.is_folder()) {
                continue;
            }

            this->scan_tree_inner(info.path(), our_index, our_depth);
        }
    };

    void FSTree::render_in_internal(
        class Rml::Element& parent, uint32_t depth, size_t element_index
    ) {}

    // Makes an element which roughly looks like
    // <button class="file_row lighter_less">
    //     <div class="file_filler" data-style-width=""></div>
    //     <div class="folder_filler">
    //         <p>+</p>
    //     </div>
    //     <span id="info"></span>
    // </button>
    class Rml::Element& FSTree::create_folder(
        std::string&& name, uint32_t depth, bool expanded, class Rml::Element& parent
    ) {

        Rml::XMLAttributes attribs = {{"class", Rml::Variant{"file_row lighter_less"}}};
        auto button = Rml::Factory::InstanceElement(&parent, "*", "button", attribs);

        auto*      button_ptr = parent.AppendChild(std::move(button));
        const auto new_width  = this->settings.per_layer_gap * static_cast<float>(depth);

        button_ptr->AppendChild(Rml::Factory::InstanceElement(
            button_ptr, "*", "div",
            {{"class", Rml::Variant{"file_filler"}},
             {"data-style-width", Rml::Variant{std::format("{}dp", new_width)}}}
        ));

        button_ptr->AppendChild([&] {
            auto ele = Rml::Factory::InstanceElement(
                button_ptr, "*", "div", {{"class", Rml::Variant{"folder_filler"}}}
            );

            Rml::Factory::InstanceElementText(ele.get(), expanded ? "-" : "+");

            return ele;
        }());

        button_ptr->AppendChild([&] {
            auto ele = Rml::Factory::InstanceElement(button_ptr, "*", "span", {});

            Rml::Factory::InstanceElementText(ele.get(), std::format("<p>{}</p>", name));

            return ele;
        }());

        return *button_ptr;
    }

    // Builds an element which represents the RML
    // <button class="file_row lighter_less">
    //     <div class="file_filler" data-style-width=""></div>
    //     <span><p>FileName</p></span>
    // </button>
    Rml::Element&
    FSTree::create_file(std::string&& name, uint32_t depth, Rml::Element& parent) {
        Rml::XMLAttributes attribs = {{"class", Rml::Variant{"file_row lighter_less"}}};
        auto button = Rml::Factory::InstanceElement(&parent, "*", "button", attribs);

        auto*      button_ptr = parent.AppendChild(std::move(button));
        const auto new_height =
            this->settings.per_layer_gap * static_cast<float>(depth) + ExpanderSize;

        button_ptr->AppendChild(Rml::Factory::InstanceElement(
            button_ptr, "*", "div",
            {{"class", Rml::Variant{"file_filler"}},
             {"data-style-width", Rml::Variant{std::format("{}dp", new_height)}}}
        ));

        button_ptr->AppendChild([&] {
            auto ele = Rml::Factory::InstanceElement(button_ptr, "*", "span", {});

            Rml::Factory::InstanceElementText(ele.get(), std::format("<p>{}</p>", name));

            return ele;
        }());

        return *button_ptr;
    }

    void FSTree::render_in(Rml::Element& parent, Rml::ElementDocument& owner_document) {}

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

    void FSTree::bind_data() {
        auto ctor = this->owner->CreateDataModel("file_browser");

        if (!ctor) {
            throw std::runtime_error(std::format("Failed to create a file_browser model"));
        }

        if (auto handle = ctor.RegisterStruct<FileTreeNode>()) {
            handle.RegisterMember("arr_index", &FileTreeNode::raw_arr_index);
            handle.RegisterMember("is_folder", &FileTreeNode::is_folder);
            handle.RegisterMember("collapsed", &FileTreeNode::is_collapsed);
            handle.RegisterMember("visible", &FileTreeNode::is_visible);
        }

        if (!ctor.RegisterArray<decltype(this->filesystem)>()) {
            Rml::Log::Message(Rml::Log::LT_WARNING, "Failed to register filesystem!");
        }

        ctor.Bind("files", &this->filesystem);
    }

} // namespace soil
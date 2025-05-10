#pragma once

#include <RmlUi/Core/Variant.h>
#include <format>
#include <rfl.hpp>
#include <stdexcept>

namespace Rml {
    class DataModelConstructor;
    class Context;
} // namespace Rml

namespace soil {
    struct SoilSettings {
        float top_bar_height{30};
        float file_browser_width{15};
        float per_layer_gap{20};

        Rml::Variant get_member(const std::string& name) {
            if (name == "top_bar_height") {
                return Rml::Variant{this->top_bar_height};
            } else {
                throw std::invalid_argument(std::format("Unknown member read: {}", name));
            }
        }

        void bind_callbacks(Rml::Context& context, class DocumentManager& manager);
        void bind_settings(Rml::DataModelConstructor& model_builder);
    };

} // namespace soil
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
        float ui_scale{1};
        float ui_scale_change_speed{0.01};

        void bind_callbacks(Rml::Context& context, class DocumentManager& manager);
        void bind_settings(Rml::DataModelConstructor& model_builder);
    };

} // namespace soil
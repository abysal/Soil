#include "./settings.hpp"
#include "document/document_manager.hpp"
#include "settings.hpp"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/DataTypes.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/Log.h>
#include <gsl/pointers>
#include <stdexcept>

namespace soil {

    void SoilSettings::bind_callbacks(Rml::Context& context, DocumentManager& manager) {

        auto soil = context.GetDataModel("core_ui");

        soil.BindEventCallback(
            "settings_pressed",
            [&](Rml::DataModelHandle, class Rml::Event&, const Rml::VariantList&) {
                auto* doc = manager.get_doc_by_title("Settings");

                if (!doc) {
                    throw std::logic_error("Tried to show settings, when they dont exist!");
                }

                doc->Show();
            }
        );

        soil.BindEventCallback(
            "close_settings",
            [&](Rml::DataModelHandle, class Rml::Event&, const Rml::VariantList&) {
                auto* doc = manager.get_doc_by_title("Settings");

                if (!doc) {
                    throw std::logic_error("Tried to show settings, when they dont exist!");
                }

                doc->Hide();
            }
        );
    }

    void SoilSettings::bind_settings(Rml::DataModelConstructor& model_builder) {
        model_builder.Bind("top_bar_height", &this->top_bar_height);
        model_builder.Bind("file_browser_width", &this->file_browser_width);
        model_builder.Bind("per_layer_gap", &this->per_layer_gap);
    }
} // namespace soil
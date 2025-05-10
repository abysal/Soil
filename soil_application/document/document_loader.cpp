#include "./document_loader.hpp"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/FileInterface.h>
#include <filesystem>
#include <format>
#include <rfl/json.hpp>
#include <stdexcept>
#include <unordered_map>

namespace soil {

    struct ToLoad {
        std::unordered_map<std::string, bool> documents{};
    };

    ToLoad read_data(Rml::FileInterface& file_interface) {
        std::string out;

        if (!file_interface.LoadFile("resources/documents_to_load.json", out)) {
            throw std::invalid_argument(std::format("Missing documents to load"));
        }

        const auto to_load = rfl::json::read<ToLoad>(out);

        if (to_load.has_value()) {
            return std::move(to_load.value());
        } else {
            throw std::invalid_argument(
                std::format("Error when parsing documents to load: {}", to_load.error().what())
            );
        }
    }

    void DocumentLoader::load_inital_documents(
        Rml::Context& ctx, Rml::FileInterface& file_interface
    ) {
        const auto to_load = read_data(file_interface);

        const std::filesystem::path base_path = "resources/";

        for (const auto& [offset_path, show] : to_load.documents) {
            const auto path = base_path / offset_path;

            auto* doc = ctx.LoadDocument(path.string());

            if (!doc) {
                throw std::invalid_argument(
                    std::format("Failed to parse document at: {}", path.string())
                );
            }

            if (show) {
                doc->Show();
            }
        }
    }

} // namespace soil
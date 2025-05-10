#include "./document_manager.hpp"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>


namespace soil {
    Rml::ElementDocument* DocumentManager::get_doc_by_title(const std::string& title) const {

        const auto doc_count = this->active_context->GetNumDocuments();

        for (int doc = 0; doc < doc_count; doc++) {

            auto* document = this->active_context->GetDocument(doc);

            if (!document) {
                continue;
            }

            if (document->GetTitle() == title) {
                return document;
            }
        }

        return nullptr;
    }
} // namespace soil
#pragma once

#include <gsl/pointers>
#include <string>

namespace Rml {
    class Context;
    class ElementDocument;
} // namespace Rml

namespace soil {
    class DocumentManager {
    public:
        DocumentManager(DocumentManager&&)                 = default;
        DocumentManager& operator=(DocumentManager&&)      = default;
        DocumentManager(const DocumentManager&)            = delete;
        DocumentManager& operator=(const DocumentManager&) = delete;
        DocumentManager(gsl::not_null<Rml::Context*> context) : active_context(context) {}
        DocumentManager() = default;

        Rml::ElementDocument* get_doc_by_title(const std::string& title) const;

    private:
        Rml::Context* active_context{nullptr};
    };
} // namespace soil
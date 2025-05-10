#pragma once

namespace Rml {
    class Context;
    class FileInterface;
} // namespace Rml

namespace soil {
    class DocumentLoader {
    public:
        void load_inital_documents(
            class Rml::Context& ctx, class Rml::FileInterface& file_interface
        );

    private:
    };
} // namespace soil
#pragma once
#include "folder_tree/folder_tree.hpp"
#include "settings.hpp"
#include <memory>

namespace Rml {
    class Context;
    class Element;
} // namespace Rml

namespace soil {
    class FsProvider {
    public:
        FsProvider(const FsProvider&)            = delete;
        FsProvider& operator=(const FsProvider&) = delete;
        FsProvider(FsProvider&&)                 = default;
        FsProvider& operator=(FsProvider&&)      = default;
        FsProvider(
            gsl::not_null<class Rml::Context*> owner_ctx,
            const std::filesystem::path& folder_to_scan, struct SoilSettings& settings
        )
            : tree(owner_ctx, folder_to_scan, settings) {}

        void render_tree(class Rml::Element& parent_div) { this->tree.render_in(parent_div); }

        static std::optional<FsProvider> poll_user(
            gsl::not_null<class Rml::Context*> owner_ctx, struct SoilSettings& settings,
            bool auto_scan
        );

    private:
        FSTree tree;
    };

    struct FsProviderSelectedFolder {
        std::optional<FsProvider> provider{};
    };

    using FsProviderPtr = std::shared_ptr<FsProvider>;
} // namespace soil
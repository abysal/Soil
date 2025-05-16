#include "./fs_provider.hpp"
#include <optional>
#include <portable-file-dialogs.h>

namespace soil {

    std::optional<FsProvider> FsProvider::poll_user(
        gsl::not_null<class Rml::Context*> owner_ctx, struct SoilSettings& settings,
        bool auto_scan
    ) {

        auto       folder = pfd::select_folder("Project Folder", "./");
        const auto path   = folder.result();

        if (path.empty()) {
            return std::nullopt;
        }

        auto provider = FsProvider{owner_ctx, std::filesystem::path(path), settings};

        if (auto_scan) {
            provider.tree.scan_tree();
        }

        return provider;
    }
} // namespace soil
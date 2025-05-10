#include "./fs_shim.hpp"
#include <RmlUi/Core/Types.h>
#include <assert.h>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <ios>
#include <spanstream>
#include <utility>
#include <variant>

#define SHIMMED_FILE(path, data) this->baked_files.insert({path, data});

namespace soil {
    FilesystemShim::FilesystemShim() {
        // Add any preloaded assets

        // NOLINTNEXTLINE
        //         char font[] = {
        // #embed "../../resources/fonts/JetBrainsMonoNL-Regular.ttf"
        //         };

        //         this->baked_files["fonts/JetBrainsMonoNL-Regular.ttf"] =
        //             std::span<char>(font, sizeof(font));

        //         char main[] = {
        // #embed "../../resources/ui/main.rml"
        //         };

        //         this->baked_files["ui/main.rml"] = std::span<char>(font, sizeof(main));
    }

    size_t FilesystemShim::Read(void* out_buffer, size_t count, Rml::FileHandle handle) {
        assert(this->file_lookup.contains(handle));

        auto& file = this->file_lookup[handle];

        if (std::holds_alternative<std::ifstream>(file)) {
            assert(false);
            return 0;
        } else {
            auto& stream = std::get<std::ispanstream>(file);

            auto& val = stream.read(
                static_cast<char*>(out_buffer), static_cast<std::streamsize>(count)
            );

            if (val.bad()) {
                assert(false);
            }

            return static_cast<size_t>(

                val.gcount()
            );
        }
    }

    bool FilesystemShim::Seek(Rml::FileHandle handle, long offset, int origin) {
        assert(this->file_lookup.contains(handle));

        auto& file = this->file_lookup[handle];

        if (std::holds_alternative<std::ifstream>(file)) {
            assert(false);
            return false;
        } else {

            const auto seek_dir = [=]() {
                switch (origin) {
                case SEEK_SET: {
                    return std::ios_base::beg;
                }
                case SEEK_CUR: {
                    return std::ios_base::cur;
                }
                case SEEK_END: {
                    return std::ios_base::end;
                }
                default: {
                    std::unreachable();
                }
                }
            }();

            auto& stream = std::get<std::ispanstream>(file);
            return !stream.seekg(offset, seek_dir).fail();
        }
    }

    Rml::FileHandle FilesystemShim::Open(const std::string& path) {
        const auto handle = this->next_handle++;

        if (!this->baked_files.contains(path)) {
            return 0;
        }

        auto stream = std::ispanstream(this->baked_files[path]);

        this->file_lookup[handle] = std::move(stream);

        return handle;
    }

    size_t FilesystemShim::Tell(Rml::FileHandle handle) {

        assert(this->file_lookup.contains(handle));

        auto& file = this->file_lookup[handle];

        if (std::holds_alternative<std::ifstream>(file)) {
            auto& stream = std::get<std::ifstream>(file);
            return static_cast<size_t>(stream.tellg());
        } else {
            auto& stream = std::get<std::ispanstream>(file);
            return static_cast<size_t>(stream.tellg());
        }
    }
} // namespace soil